#include <gf/Clock.h>
#include <gf/Color.h>
#include <gf/Event.h>
#include <gf/RenderWindow.h>
#include <gf/Shapes.h>
#include <gf/Vector.h>
#include <gf/Window.h>
#include <gf/Queue.h>
#include <gf/Action.h>
#include <gf/EntityContainer.h>
#include <gf/UI.h>
#include <gf/ViewContainer.h>
#include <gf/Views.h>

#include <gf/Sleep.h>
#include <gf/Time.h>

#include "message.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <string>

#include <thread>
#include <sstream>

#define DEFAULT_WIDTH 960
#define DEFAULT_HEIGHT 900

#include "c_grid.h"

#include "packet.h"

using boost::asio::ip::tcp;

int connection(tcp::socket** socket, char *ip, char *port) {
  boost::asio::io_service io_service;
  tcp::resolver resolver(io_service);

  tcp::resolver::query query(ip, port);

  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  tcp::resolver::iterator end;

  *socket = new tcp::socket(io_service);
  boost::system::error_code error = boost::asio::error::host_not_found;

  while(error && endpoint_iterator != end) {
    (*socket)->close();
    (*socket)->connect(*endpoint_iterator++, error);
  }

  if(error) {
    throw boost::system::system_error(error);
  }
}

enum class State {
  WaitPlayer,
  WaitUpdate,
  WaitAnswer,
  FatalError,
  WaitUpdateAnswer,
  WaitUpdateAfterAnswer,
  Play,
  Exit,
  Connexion,
  Connected,
  Placing,
  PlacingOver,
  WaitPlaySignal,
  Win,
  Lose
};

enum class CustomError {
  None,
  NotFinished,
  Refused,
  WrongPlacing
};

// Thread qui va communiquer avec le serveur
void reception_thread(char *ip, char *port, tcp::socket* socket, gf::Queue<Message>* messages) {
    bool fatalError = false;
    while( !fatalError ) {
      fatalError = !get_message(*socket, *messages);
    }
}

/**
 * Affiche la fenetre de 'pause' (quand on appuie sur ESC)
 * 
 * @return bool true si l'utilisateur a demandé à quitter
 */
bool escFct(tcp::socket* socket, gf::RenderWindow &renderer, gf::UI &ui, bool &aide) {
  bool ret = false;

  // Afficher la fenêtre d'UI
  if(ui.begin("Menu", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Title)) {

    ui.layoutRowDynamic(25, 1);

    if(ui.buttonLabel("Aide")) {
      aide = true;
    }

    if(ui.buttonLabel("Quitter")) {
      // Envoi d'un message de déconnexion au serveur
      send_message(*socket, create_quit_message());
      ret = true;
    }
  }

  ui.end();

  renderer.draw(ui);

  return ret;
}

/**
 * Affiche les UIs nécessaires en fonction de la valeur de state
 * 
 * @return bool true si l'utilisateur a demandé à quitter
 */
bool displayStateUi(gf::Window &window, gf::RenderWindow &renderer, gf::UI &ui, State state) {
  bool ret = false;

  gf::RectF rect(DEFAULT_GRID_X, 0, 640, 100);

  if(state == State::Win) {
    if(ui.begin("Victoire !", rect, gf::UIWindow::Title)) {

      ui.layoutRowDynamic(25, 1);

      ui.label("Félicitations, vous avez gagné !");
    }

    ui.end();
    renderer.draw(ui);
  } else if(state == State::Lose) {
    if(ui.begin("Défaite...", rect, gf::UIWindow::Title)) {

      ui.layoutRowDynamic(25, 1);

      ui.label("Dommage, vous avez perdu");
    }

    ui.end();
    renderer.draw(ui);
  }

  if(state == State::WaitAnswer) {
    if(ui.begin("Pause", rect, gf::UIWindow::Title)) {

      ui.layoutRowDynamic(25, 1);

      ui.label("Attente de la confirmation du serveur");
    }

    ui.end();
    renderer.draw(ui);
  }

  if(state == State::WaitUpdate || state == State::WaitPlayer) {
    if(ui.begin("Pause", rect, gf::UIWindow::Title)) {

      ui.layoutRowDynamic(25, 1);

      if(state == State::WaitPlayer) {
        ui.label("Attente de l'autre joueur");
      } else {
        ui.label("Votre adversaire joue");
      }
    }

    ui.end();
    renderer.draw(ui);
  }

  if(state == State::FatalError) {
    // Afficher la fenêtre d'UI
    if(ui.begin("Erreur", rect, gf::UIWindow::Title)) {

      ui.layoutRowDynamic(25, 1);

      ui.label("Le serveur a rencontré une erreur");

      if(ui.buttonLabel("OK (quitter)")) {
        ret = true;
      }
    }

    ui.end();
    renderer.draw(ui);
  }

  return ret;
}

int main(int argc, char *argv[]) {

  srand(time(NULL));

  // Initialisation
  static constexpr gf::Vector2u ScreenSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  gf::Window window("Client Stratego", ScreenSize);
  window.setFramerateLimit(60);
  gf::RenderWindow renderer(window);

  // Actions
  gf::ActionContainer actions;

  gf::Action closeWindowAction("Close window");
  closeWindowAction.addCloseControl();
  actions.addAction(closeWindowAction);

  gf::Action escAction("Echap");
  escAction.addKeycodeKeyControl(gf::Keycode::Escape);
  actions.addAction(escAction);

  // Resource manager
  gf::ResourceManager resources;
  resources.addSearchDir(".");

  gf::EntityContainer entities;

  // Grille du jeu
  Grid g(resources);
  g.setPosition({DEFAULT_GRID_X, DEFAULT_GRID_Y});
  entities.addEntity(g);

  Selection s(resources);
  s.setPosition({DEFAULT_SELECT_X, DEFAULT_SELECT_Y});
  entities.addEntity(s);

  // Boucle de jeu
  renderer.clear(gf::Color::White);

  gf::Clock clock;

  gf::Font font;
  bool loaded = font.loadFromFile("16_DejaVuSans.ttf");
  if(!loaded) {
    return EXIT_FAILURE;
  }

  gf::UI ui(font);

  State state = State::Connexion;

  gf::Queue<Message> messages;
  tcp::socket *socket;

  gf::RectF world(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);
  gf::ViewContainer views;
  gf::ScreenView screenView;
  views.addView(screenView);
  gf::ExtendView extendView(world);
  views.addView(extendView);
  views.setInitialScreenSize({DEFAULT_WIDTH, DEFAULT_HEIGHT});

  gf::RectF extendedWorld = world.extend(1000);
  gf::RectangleShape background(extendedWorld);
  gf::Texture background_texture;
  background_texture.loadFromFile("fond.jpg");
  background.setTexture(background_texture);

  /******************************************************************/
  /***
  /***
  /***             1 ERE BOUCLE DE JEU
  /***
  /***
  /******************************************************************/

  static char servIp[16];
  static std::size_t servIpLength;
  static char servPort[6];
  static std::size_t servPortLength;

  bool displayEscUi = false;
  bool escPressed = false;
  bool error = false;

  // Première boucle : sélection du serveur
  while(state != State::Connected && window.isOpen()) {
    gf::Event event;

    while(window.pollEvent(event)) {
      actions.processEvent(event);
      ui.processEvent(event);
      views.processEvent(event);
    }

    if(closeWindowAction.isActive()) {
      state = State::Exit;
    }

    if(escAction.isActive()) {
      if(!escPressed) {
        displayEscUi = !displayEscUi;
      }
      escPressed = true;
    } else {
      escPressed = false;
    }

    // Update
    gf::Time time = clock.restart();

    // Draw
    renderer.clear();

    renderer.setView(screenView);

    renderer.draw(background);

    // UI
    if(displayEscUi) {
      // Afficher la fenêtre d'UI
      if(ui.begin("Menu", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Minimizable | gf::UIWindow::Title)) {

        ui.layoutRowDynamic(25, 1);

        if(ui.buttonLabel("Quitter")) {
          state = State::Exit;
        }
      }

      ui.end();
    }

    // Fenetre de selection du serveur
    // Afficher la fenêtre d'UI
    if(ui.begin("Serveur", gf::RectF(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT), gf::UIWindow::Border | gf::UIWindow::Title)) {

      ui.layoutRowDynamic(30, 1);

      ui.label("IP du serveur :");
      ui.edit(gf::UIEditType::Simple, servIp, servIpLength, gf::UIEditFilter::Default);

      ui.label("Port du serveur :");
      ui.edit(gf::UIEditType::Simple, servPort, servPortLength, gf::UIEditFilter::Decimal);

      if(ui.buttonLabel("Confirmer")) {
        servIp[servIpLength] = '\0';

        try {
          connection(&socket, servIp, servPort);

          // Création du thread qui va se connecter au serveur
          std::thread rt(reception_thread, servIp, servPort, socket, &messages);

          rt.detach();

          state = State::Connected;

        } catch(std::exception &e) {
          state = State::FatalError;
        }
      }

      if(error) {
        ui.label("Impossible de se connecter au serveur");
      }
    }
    ui.end();

    if(state == State::Exit) {
      window.close();
    }

    renderer.draw(ui);

    renderer.display();
  }

  if(state == State::Exit || state == State::FatalError) {
    return 0;
  }

  servIp[servIpLength] = '\0';
  displayEscUi = false;
  escPressed = false;

  gf::Action validAction("Validation");
  validAction.addKeycodeKeyControl(gf::Keycode::Return);
  validAction.setInstantaneous();
  actions.addAction(validAction);

  gf::Action placementAleatoireAction("Placement aléatoire");
  placementAleatoireAction.addKeycodeKeyControl(gf::Keycode::A);
  placementAleatoireAction.setInstantaneous();
  actions.addAction(placementAleatoireAction);

  /******************************************************************/
  /***
  /***
  /***             2 EME BOUCLE DE JEU
  /***
  /***
  /******************************************************************/

  state = State::Placing;

  // Erreur de placement des pièces (-1 si aucune)
  CustomError customError = CustomError::None;

  bool uiOpen = false; // Pour que si une fenêtre est ouverte, on ne puisse pas cliquer autre part
  bool aide = false;

  while(window.isOpen() && state != State::PlacingOver) {
      gf::Event event;

      // Entrées
      while(window.pollEvent(event)) {
        actions.processEvent(event);

        if(event.type == gf::EventType::MouseMoved) {
          s.updateMouseCoords(renderer.mapPixelToCoords(event.mouseCursor.coords));
        }

        if(event.type == gf::EventType::MouseButtonPressed && state == State::Placing && !displayEscUi && !aide) {

          // Clic gauche : choisir une pièce
          if(event.mouseButton.button == gf::MouseButton::Left) {
            // Récupération de la pièce dans le selecteur
            gf::Vector2i c = s.getPieceCoordsFromMouse(renderer.mapPixelToCoords(event.mouseButton.coords));
            if(c.x != -1 && c.y != -1) {
              Piece p = s.getPiece({(unsigned)c.x, (unsigned)c.y});
              // On sélectionne la pièce voulue
              s.selectPiece((unsigned int)p.rank);
            }

            // Récupération de la case de la grille
            c = g.getPieceCoordsFromMouse(renderer.mapPixelToCoords(event.mouseButton.coords));
            if(c.x != -1 && c.y != -1) {
              // Vérification qu'une pièce est selectionné dans le sélecteur
              if(s.selected != -1) {
                // On peut alors regarder la case cliquée
                Piece p = g.getPiece({(unsigned)c.x, (unsigned)c.y});

                Piece newPiece;
                newPiece.rank = Rank(s.selected);
                newPiece.side = Side::Red;

                if(g.setPiece(c, newPiece)) {
                  s.takeOnePiece(s.selected);
                  s.selected = -1;
                } else {
                  // On tente de récupérer la pièce qui est dans la case pour l'échanger
                  if(g.changePiece(c, newPiece)) {
                    s.takeOnePiece(s.selected);
                    s.addPiece(newPiece);
                    s.selected = (int)(newPiece.rank);
                  }
                }
              } else { // Pas de pièce sélectionnée, on déplace la pièce qui était à cette case

                // Suppression de la pièce
                Piece p = g.getPiece({(unsigned)c.x, (unsigned)c.y});

                if(p.side != Side::Blue && p.rank != Rank::Empty && p.rank != Rank::Water) {
                  // On peut supprimer la pièce de la grille
                  g.removePiece({(unsigned)c.x, (unsigned)c.y});

                  // On ajoute maintenant cette pièce au selecteur
                  s.addPiece(p);
                }

                s.selectPiece((unsigned int)p.rank);
              }
            }
          }

          // Clic droit : supprimer une pièce de la grille
          if(event.mouseButton.button == gf::MouseButton::Right) {
            // Récupération de la case de la grille
            gf::Vector2i c = g.getPieceCoordsFromMouse(renderer.mapPixelToCoords(event.mouseButton.coords));
            if(c.x != -1 && c.y != -1) {
              // On retire la pièce de la grille
              Piece p = g.getPiece({(unsigned)c.x, (unsigned)c.y});

              if(p.rank != Rank::Empty && p.rank != Rank::Water) {
                // On peut supprimer la pièce de la grille
                g.removePiece({(unsigned)c.x, (unsigned)c.y});

                // On ajoute maintenant cette pièce au selecteur
                s.addPiece(p);
              }
            }
          }
        }

        views.processEvent(event);
        ui.processEvent(event);
      }

      if(closeWindowAction.isActive()) {
        // Envoi d'un message de déconnexion au serveur
        send_message(*socket, create_quit_message());
        state = State::Exit;
      }

      if(escAction.isActive()) {
        if(aide) {
          aide = false;
        } else {
          if(!escPressed) {
            displayEscUi = !displayEscUi;
          }
          escPressed = true;
        }
      } else {
        escPressed = false;
      }

      if(placementAleatoireAction.isActive()) {
        // Placement aléatoire de toutes les pièces restantes
        
        Piece p = s.getRandomPiece();
        while(p.rank != Rank::Empty) {
          g.setPieceRandom(p);
          s.takeOnePiece((int)p.rank);
          
          p = s.getRandomPiece();
        }
      }

      if(validAction.isActive()) {
        // On appuie sur entrer pour envoyer les pièces au serveur
        if(!s.isEmpty()) {
          // Il reste des pièces à placer
          customError = CustomError::NotFinished;
        } else if(state != State::WaitAnswer) {
          // Envoi des pièces au serveur
          Initialize init;

          int nb = PLAYER_MAX_PIECES - 1;
          // Envoi des pièces (4 dernières lignes)
          for(int y = 6; y <= 9; y++) {
            for(int x = 0; x < 10; x++) {
              gf::Vector2u coords = {x, y};
              init.pieces[nb--] = create_resumed_piece(&coords, (int)(g.getPiece(coords).rank), false);
            }
          }
          // Envoi du message
          send_message(*socket, create_initiate_message(init));

          // On attend maintenant la réponse :
          state = State::WaitAnswer;
        }
      }

      // Update

      gf::Time time = clock.restart();
      g.update(time);

      // Draw
      renderer.clear();
      renderer.setView(extendView);
      renderer.draw(background);
      entities.render(renderer);
      
      if(aide) {
        gf::Sprite help_sprite;
        help_sprite.setTexture(resources.getTexture("help.png"));
        help_sprite.setPosition({DEFAULT_GRID_X, DEFAULT_GRID_Y});
        renderer.draw(help_sprite);
      }

      // Réception des messages
      Message msg;
      bool msgLu = messages.poll(msg);

      if(msgLu) {
        switch(msg.id) { // Type du message
          case ID_message::Error:
            // Erreur provenant du serveur
            state = State::FatalError;
            break;
          case ID_message::Accept: // Acceptation du serveur
            if(state != State::WaitAnswer) {
              if(!msg.data.accept) {
                customError = CustomError::Refused;
              }
            } else {
              if(!msg.data.accept) {
                state = State::Placing;
                customError = CustomError::WrongPlacing;
              } else {
                state = State::PlacingOver;
              }
            }
            break;
          default:
            break;
        }
      }

      renderer.setView(screenView);
      // UI
      if(state == State::FatalError) {
        // Afficher la fenêtre d'UI
        if(ui.begin("Erreur", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Title)) {

          ui.layoutRowDynamic(25, 1);

          ui.label("Le serveur a rencontré une erreur");

          if(ui.buttonLabel("OK (quitter)")) {
            window.close();
          }
        }

        ui.end();
        renderer.draw(ui);
      }

      // UI
      if(displayEscUi && !aide) {
        if(escFct(socket, renderer, ui, aide)) {
          // Il va falloir quitter
          state = State::Exit;
        }
      }

      if(customError != CustomError::None) {
        if(ui.begin("Erreur", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Minimizable | gf::UIWindow::Title)) {

          ui.layoutRowDynamic(25, 1);

          switch(customError) {
            case CustomError::NotFinished:
              ui.label("Il reste des pièces à placer");
              break;
            case CustomError::Refused:
              ui.label("Vous avez été refusé par le serveur");
              break;
            case CustomError::WrongPlacing:
              ui.label("Le placement des pièces est incorrect");
              break;
            default:
              ui.label("Une erreur est survenue");
              break;
          }

          if(ui.buttonLabel("OK")) {
            customError = CustomError::None;
          }

          ui.end();

          renderer.draw(ui);
        }
      }

      renderer.setView(extendView);

      if(displayStateUi(window, renderer, ui, state)) {
        state = State::Exit;
      }

      if(state == State::Exit) {
        window.close();
      }

      renderer.display();
  }

  if(state == State::Exit || state == State::FatalError) {
    return 0;
  }

  /******************************************************************/
  /***
  /***
  /***             3 EME BOUCLE DE JEU
  /***
  /***
  /******************************************************************/

  // Nos pièces restantes
  Selection our_s(resources);
  our_s.setPosition({DEFAULT_FIRST_BAR_X, DEFAULT_FIRST_BAR_Y});
  our_s.makeVertical();
  entities.addEntity(our_s);

  // Les pièces restantes de l'ennemi
  Selection your_s(resources);
  your_s.setPosition({DEFAULT_SECOND_BAR_X, DEFAULT_SECOND_BAR_Y});
  your_s.makeVertical();
  your_s.makeBlueSide();
  entities.addEntity(your_s);

  entities.removeEntity(&s);

  state = State::WaitPlayer;

  bool waitForPlayer = true;
  bool pause = true;
  bool waitForConfirm = false;

  // Boucle principale de jeu
  while(window.isOpen()) {
    gf::Event event;

    // Entrées
    while(window.pollEvent(event)) {
      actions.processEvent(event);
      ui.processEvent(event);
      views.processEvent(event);

      if(event.type == gf::EventType::MouseButtonPressed) {

        if(event.mouseButton.button == gf::MouseButton::Left) {
            if(state != State::Play) {
              continue;
            }

            gf::Vector2i coords = g.getPieceCoordsFromMouse(renderer.mapPixelToCoords(event.mouseButton.coords));
            if(coords.x != -1 && coords.y != -1) {
              if(g.isValidMove(coords)) {
                // Envoi au serveur du mouvement
                gf::Vector2u source = g.selected;
                gf::Vector2u target = coords;
                send_message(*socket, create_move_message(create_movement(&source, &target, false)));
                
                state = State::WaitUpdateAnswer;
              } else {
                g.selectPiece({(unsigned)coords.x, (unsigned)coords.y});
              }
            }
        }
      }
    }

    if(closeWindowAction.isActive() && state != State::FatalError) {
      // Envoi d'un message de déconnexion au serveur
      send_message(*socket, create_quit_message());
      window.close();
    }

    if(escAction.isActive()) {
      if(!escPressed) {
        displayEscUi = !displayEscUi;
      }
      escPressed = true;
    } else {
      escPressed = false;
    }

    // Update

    gf::Time time = clock.restart();
    g.update(time);

    // Draw
    renderer.clear();
    renderer.setView(extendView);
    renderer.draw(background);
    entities.render(renderer);

    if(aide) {
      gf::Sprite help_sprite;
      help_sprite.setTexture(resources.getTexture("help.png"));
      help_sprite.setPosition({DEFAULT_GRID_X, DEFAULT_GRID_Y});
      renderer.draw(help_sprite);
    }

    // Réception des messages
    Message msg;
    bool msgLu = messages.poll(msg);

    if(msgLu && state != State::FatalError && state != State::Win && state != State::Lose) {
      switch(msg.id) { // Type du message
        case ID_message::Error:
          // Erreur provenant du serveur
          state = State::FatalError;
          break;
        case ID_message::Accept: // Acceptation du serveur
          if(!msg.data.accept) {
            // Le mouvement n'a pas été accepté
            state = State::FatalError;
          } else if(state == State::WaitAnswer) {
            state = State::WaitUpdate;
          } else if(state == State::WaitUpdateAnswer) {
            state = State::WaitUpdateAfterAnswer;
          }
          break;
        case ID_message::Play: // Jouer
          if(state == State::WaitPlayer) {
            state = State::WaitUpdate;
          } else {
            state = State::Play;
          }
          break;
        case ID_message::Update: // Update
          if(state != State::WaitUpdate && state != State::WaitUpdateAfterAnswer) {
            state = State::FatalError;
            break;
          }

          if(state == State::WaitUpdate) {
            state = State::WaitPlaySignal;
          } else {
            state = State::WaitUpdate;
          }

          gf::Vector2u firstCoords;
          get_vector_coord(&firstCoords, msg.data.update.movement.source, true);

          gf::Vector2u lastCoords;
          get_vector_coord(&lastCoords, msg.data.update.movement.target, true);

          if(!msg.data.update.collision) {
            // Si le serveur indique qu'il n'y a pas eu de collision
            // On peut alors effectuer le mouvement sans problème
            if(!g.movePieceTo(firstCoords, lastCoords, true)) {
              state = State::FatalError;
            }
          } else {
            // Le serveur précise qu'il y a eu collision (combat) entre deux pièces
            
            // lastPieceBefore -> la valeur de la pièce ennemie
            Piece lastPieceBefore;
            lastPieceBefore.rank = (Rank)( msg.data.update.enemy_value );
            lastPieceBefore.side = Side::Other; // On ne sait pas : l'inverse de firstPiece

            if(!g.makeUpdate(firstCoords, lastCoords, lastPieceBefore, msg.data.update.result, our_s, your_s)) {
              state = State::FatalError;
            }
          }

          g.selected = {-1, -1};

          break;
        case ID_message::End: // Signal de fin du jeu
          state = msg.data.end == Result::Win ? State::Win : State::Lose;
          break;
      }
    }

    // UI
    renderer.setView(screenView);
    if(displayEscUi) {
      if(escFct(socket, renderer, ui, aide)) {
        state = State::Exit;
      }
    }

    renderer.setView(extendView);
    if(displayStateUi(window, renderer, ui, state)) {
      state = State::Exit;
    }

    if(state == State::Exit) {
      window.close();
    }

    renderer.display();
  }

  return 0;
}
