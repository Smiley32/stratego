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

double get_current_scale(gf::Window &window) {
  return 1;
  
}

/// Calcule la position en fonction de scale
gf::Vector2f get_current_position(gf::Vector2u default_pos, double scale) {
  return {(float)(scale * default_pos.x), (float)(scale * default_pos.y)};
}

#include "c_grid.h"

#include "packet.h"

using boost::asio::ip::tcp;


// Récupération d'un message du serveur
boost::array<char, 128> get_message(tcp::socket *socket, size_t *length) {
  boost::array<char, 128> buf;
  boost::system::error_code error;

  *length = socket->read_some(boost::asio::buffer(buf), error);

  if(error) {
    buf[0] = -1;
  }

  return buf;
}

/*
void send_packet(tcp::socket* socket, Packet &p) {
  char *data = (char*)p.getData();
  std::cout << "Message envoyé : ";
  for(int i = 0; i < p.getDataSize(); i++) {
    std::cout << (int)data[i] << ";";
  }
  std::cout << std::endl;
  
  boost::system::error_code ignored_error;
  boost::asio::write(*socket, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
}*/

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
      fatalError = get_message(*socket, *messages);
    }
}

/**
 * Affiche la fenetre de 'pause' (quand on appuie sur ESC)
 * 
 * @return bool true si l'utilisateur a demandé à quitter
 */
bool escFct(tcp::socket* socket, gf::RenderWindow &renderer, gf::UI &ui) {
  bool ret = false;

  // Afficher la fenêtre d'UI
  if(ui.begin("Menu", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Title)) {

    ui.layoutRowDynamic(25, 1);

    if(ui.buttonLabel("Quitter")) {
      // Envoi d'un message de déconnexion au serveur
      /*Packet p;
      p.append(6); // Le client quitte
      send_packet(socket, p);*/
      // window.close();
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
  gf::Window window("Petit jeu en réseau (client)", ScreenSize);
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
  g.createGrid();
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
    // std::cout << "Impossible de charger la police" << std::endl;
    return EXIT_FAILURE;
  }

  gf::UI ui(font);

  State state = State::Connexion;

  gf::Queue<Message> messages;
  tcp::socket *socket;

  gf::RectF world(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);
  gf::ViewContainer views;
  gf::ScreenView screenView;
  gf::FitView fitView(world);
  views.addView(screenView);
  views.addView(fitView);
  views.setInitialScreenSize({DEFAULT_WIDTH, DEFAULT_HEIGHT});

  
  gf::RectangleShape background(world);
  background.setColor(gf::Color::Red);

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

      if(event.type == gf::EventType::MouseMoved) {
        std::cout << "(x,y): (" << event.mouseCursor.coords.x << "," << event.mouseCursor.coords.y << ")" << std::endl;
        std::cout << "map(x,y): (" << renderer.mapPixelToCoords(event.mouseCursor.coords).x << "," << renderer.mapPixelToCoords(event.mouseCursor.coords).y << ")" << std::endl;
        
      }

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
        // TODO : vérification des données entrées
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
  // std::cout << "serv ip : " << servIp << "(" << servIpLength << ")" << " port : " << servPort << std::endl;

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

  while(window.isOpen() && state != State::PlacingOver) {
      gf::Event event;

      // Entrées
      while(window.pollEvent(event)) {
        actions.processEvent(event);

        if(event.type == gf::EventType::MouseMoved) {
          // std::cout << "(x,y): (" << event.mouseCursor.coords.x << "," << event.mouseCursor.coords.y << ")" << std::endl;
          s.updateMouseCoords(renderer.mapPixelToCoords(event.mouseCursor.coords));
        }

        if(event.type == gf::EventType::MouseButtonPressed && state == State::Placing && !displayEscUi) {

          // Clic gauche : choisir une pièce
          if(event.mouseButton.button == gf::MouseButton::Left) {
            // Récupération de la pièce dans le selecteur
            gf::Vector2i c = s.getPieceCoordsFromMouse(renderer.mapPixelToCoords(event.mouseButton.coords));
            // std::cout << "Case : ( " << c.x << " , " << c.y << " )" << std::endl;
            if(c.x != -1 && c.y != -1) {
              Piece p = s.getPiece({(unsigned)c.x, (unsigned)c.y});
              // std::cout << "Piece : " << static_cast<int>(p.rank) << std::endl;
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
                // std::cout << "Piece : " << static_cast<int>(p.rank) << std::endl;

                Piece newPiece;
                newPiece.rank = Rank(s.selected);
                newPiece.side = Side::Red;

                if(g.setPiece(c, newPiece)) {
                  s.takeOnePiece(s.selected);
                  s.selected = -1;
                }/* else {
                  std::cout << "Erreur" << std::endl;
                }*/
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
        /*Packet p;
        p.append(6); // Le client quitte
        send_packet(socket, p);*/
        send_message(*socket, create_quit_message());
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

      if(placementAleatoireAction.isActive()) {
        // Placement aléatoire de toutes les pièces restantes
        
        Piece p = s.getRandomPiece();
        while(p.rank != Rank::Empty) {
          // std::cout << "Une pièce en plus !" << std::endl;
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
        } else {
          // Envoi des pièces au serveur
          // Packet p;
          // Id du message
          // p.append(1);

          Initialize init;

          int nb = PLAYER_MAX_PIECES - 1;
          // Envoi des pièces (4 dernières lignes)
          for(int y = 6; y <= 9; y++) {
            for(int x = 0; x < 10; x++) {
              gf::Vector2u coords = {x, y};
              // std::cout << "(" << x << "," << y << ") - no " << (int)(g.getPiece(coords).rank) << std::endl;
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
      renderer.setView(fitView);
      entities.render(renderer);

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
      if(displayEscUi) {
        if(escFct(socket, renderer, ui)) {
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

      renderer.setView(fitView);

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
            // g.getPieceCoordsFromMouse(event.mouseCursor.coords);
            if(state != State::Play) {
              continue;
            }

            // std::cout << "clic !" << std::endl;
            gf::Vector2i coords = g.getPieceCoordsFromMouse(renderer.mapPixelToCoords(event.mouseButton.coords));
            if(coords.x != -1 && coords.y != -1) {
              if(g.isValidMove(coords)) {
                // Envoi au serveur du mouvement
                gf::Vector2u source = g.selected;
                gf::Vector2u target = coords;
                send_message(*socket, create_move_message(create_movement(&source, &target, false)));

                /*Packet p;
                p.append(3);
                // g.getPiece({g.GridSize - (i % g.GridSize) - 1, g.GridSize - (i / g.GridSize) - 1}
                p.append((g.GridSize - g.selected.y - 1) * g.GridSize + (g.GridSize - g.selected.x - 1));
                p.append((g.GridSize - coords.y - 1) * g.GridSize + (g.GridSize - coords.x - 1));
                send_packet(socket, p);*/
                
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
      /*Packet p;
      p.append(6); // Le client quitte
      send_packet(socket, p);*/
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
    // g.update_scale(get_current_scale(window));
    // s.update_scale(get_current_scale(window));
    // our_s.update_scale(get_current_scale(window));
    // your_s.update_scale(get_current_scale(window));

    gf::Time time = clock.restart();
    g.update(time);
    // entities.update(time);

    // Draw
    renderer.clear();
    renderer.setView(fitView);
    entities.render(renderer);

    // Réception des messages
    Message msg;
    bool msgLu = messages.poll(msg);

    if(msgLu && state != State::FatalError) {
      // std::cout << "Message lu : " << (int)msg[0] << std::endl;
      switch(msg.id) { // Type du message
        case ID_message::Error:
          // Erreur provenant du serveur
          state = State::FatalError;
          // std::cout << "L'erreur vient du serveur" << std::endl;
          break;
        case ID_message::Accept: // Acceptation du serveur
          if(!msg.data.accept) {
            // Le mouvement n'a pas été accepté
            state = State::FatalError;
            // std::cout << "Le mouvement a été refusé par le serveur" << std::endl;
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
            // std::cout << "Le message update est arrivé alors qu'il n'étais pas attendu" << std::endl;
            break;
          }

          if(state == State::WaitUpdate) {
            state = State::WaitPlaySignal;
          } else {
            state = State::WaitUpdate;
          }

          gf::Vector2u firstCoords;
          get_vector_coord(&firstCoords, msg.data.update.movement.source, true);
          /*firstCoords.x = msg[2] % g.GridSize;
          firstCoords.y = (int)(msg[2] / g.GridSize);*/

          gf::Vector2u lastCoords;
          get_vector_coord(&lastCoords, msg.data.update.movement.target, true);
          /*lastCoords.x = msg[3] % g.GridSize;
          lastCoords.y = (int)(msg[3] / g.GridSize);*/

          if(!msg.data.update.collision) {
            // Si le serveur indique qu'il n'y a pas eu de collision
            // On peut alors effectuer le mouvement sans problème
            if(!g.movePieceTo(firstCoords, lastCoords, true)) {
              state = State::FatalError;
              // std::cout << "g.movePieceTo a échoué" << std::endl;
            }
          } else {
            // Le serveur précise qu'il y a eu collision (combat) entre deux pièces
            
            // lastPieceBefore -> la valeur de la pièce ennemie
            Piece lastPieceBefore;
            lastPieceBefore.rank = (Rank)( msg.data.update.enemy_value );
            lastPieceBefore.side = Side::Other; // On ne sait pas : l'inverse de firstPiece
            
            
            // int win = (int)(msg[5]); // 0 -> lose ; 1 -> win ; 2 -> draw

            if(!g.makeUpdate(firstCoords, lastCoords, lastPieceBefore, msg.data.update.result, our_s, your_s)) {
              state = State::FatalError;
              // std::cout << "g.makeUpdate a échoué" << std::endl;
            }

            // TODO: faire fonctionner les barres de pièces restantes
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
      if(escFct(socket, renderer, ui)) {
        state = State::Exit;
      }
    }

    renderer.setView(fitView);
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
