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

#include <gf/Sleep.h>
#include <gf/Time.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <string>

#include <thread>
#include <sstream>

#include "c_grid.h"

#include "packet.h"

using boost::asio::ip::tcp;

tcp::socket *sock;
gf::Queue<boost::array<char, 128>> messages;

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

void send_packet(Packet &p) {
  boost::system::error_code ignored_error;
  boost::asio::write(*sock, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
}

int connection(char *ip, char *port) {
  boost::asio::io_service io_service;
  tcp::resolver resolver(io_service);

  tcp::resolver::query query(ip, port);

  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  tcp::resolver::iterator end;

  sock = new tcp::socket(io_service);
  boost::system::error_code error = boost::asio::error::host_not_found;

  while(error && endpoint_iterator != end) {
    sock->close();
    sock->connect(*endpoint_iterator++, error);
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
  Play
};

// Thread qui va communiquer avec le serveur (le parmaètre est juste un test)
void reception_thread(char *ip, char *port) {
    bool fatalError = false;
    while( !fatalError ) {
      size_t readLength;
      boost::array<char, 128> msg = get_message(sock, &readLength);
      std::cout << "Id du message : " << (int)msg[0] << " ; taille " << readLength << std::endl;

      // Taille attendue des messages
      size_t length;
      bool continuer = true;
      do {
        switch(msg[0]) {
          case -1:
            fatalError = true;
            break;
          case 0:
          case 5:
            length = 2;
            break;
          case 1:
          case 3:
            length = 3;
            break;
          case 4:
            length = 5;
            break;
          default:
            length = 1;
            break;
        }

        std::cout << "Ajout du message" << (int)msg[0] << std::endl;
        messages.push(msg);

        continuer = false;
        if(readLength > length) {
          std::cout << "Il y a une concaténation..." << std::endl;
          // On décale msg de la longueur de length
          for(size_t i = length; i < 128; i++) {
            msg[i - length] = msg[i];
          }
          continuer = true;
          readLength -= length;
        }
        
      } while(continuer && !fatalError);
    }
}

int main(int argc, char *argv[]) {

  srand (time(NULL));

  // Initialisation
  static constexpr gf::Vector2u ScreenSize(768, 700);
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
  g.setPosition({32, 0});
  entities.addEntity(g);

  Selection s(resources);
  s.setPosition({0, g.getPosition().y + (10 * g.TileSize)});
  entities.addEntity(s);

  // Boucle de jeu
  renderer.clear(gf::Color::White);

  gf::Clock clock;

  gf::Font font;
  bool loaded = font.loadFromFile("16_DejaVuSans.ttf");
  if(!loaded) {
    std::cout << "Impossible de charger la police" << std::endl;
    return EXIT_FAILURE;
  }

  gf::UI ui(font);

  static char servIp[16];
  static std::size_t servIpLength;
  static char servPort[6];
  static std::size_t servPortLength;

  bool displayEscUi = false;
  bool escPressed = false;
  bool error = false;

  // Première boucle : sélection du serveur
  bool servSelected = false;
  while(!servSelected) {
    gf::Event event;

    while(window.pollEvent(event)) {
      actions.processEvent(event);
      ui.processEvent(event);
    }

    if(closeWindowAction.isActive()) {
      servSelected = true;
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

    // Draw
    renderer.clear();

    // UI
    if(displayEscUi) {
      // Afficher la fenêtre d'UI
      if(ui.begin("Menu", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Minimizable | gf::UIWindow::Title)) {

        ui.layoutRowDynamic(25, 1);

        if(ui.buttonLabel("Quitter")) {
          window.close();
          servSelected = true;
        }
      }

      ui.end();
    }

    // Fenetre de selection du serveur
    // Afficher la fenêtre d'UI
    if(ui.begin("Serveur", gf::RectF(0, 0, renderer.getSize().x, renderer.getSize().y), gf::UIWindow::Border | gf::UIWindow::Minimizable | gf::UIWindow::Title)) {

      ui.layoutRowDynamic(30, 1);

      ui.label("IP du serveur :");
      ui.edit(gf::UIEditType::Simple, servIp, servIpLength, gf::UIEditFilter::Default);

      ui.label("Port du serveur :");
      ui.edit(gf::UIEditType::Simple, servPort, servPortLength, gf::UIEditFilter::Decimal);

      if(ui.buttonLabel("Confirmer")) {
        // TODO : vérification des données entrées
        servIp[servIpLength] = '\0';

        try {
          connection(servIp, servPort);

          // Création du thread qui va se connecter au serveur
          std::thread rt(reception_thread, servIp, servPort);

          rt.detach();

          servSelected = true;

        } catch(std::exception &e) {
          std::cout << e.what() << std::endl;

          error = true;
        }
      }

      if(error) {
        ui.label("Impossible de se connecter au serveur");
      }
    }
    ui.end();

    renderer.draw(ui);

    renderer.display();
  }

  if(!window.isOpen()) {
    return 0;
  }

  servIp[servIpLength] = '\0';
  std::cout << "serv ip : " << servIp << "(" << servIpLength << ")" << " port : " << servPort << std::endl;

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

  // Erreur de placement des pièces (-1 si aucune)
  int errorNb = -1;

  bool fatalError = false;

  bool waitForAnswer = false;

  bool uiOpen = false; // Pour que si une fenêtre est ouverte, on ne puisse pas cliquer autre parts

  bool setupFinished = false;
  while(window.isOpen() && !setupFinished) {
      gf::Event event;

      // Entrées
      while(window.pollEvent(event)) {
        actions.processEvent(event);

        if(event.type == gf::EventType::MouseMoved) {
          // std::cout << "(x,y): (" << event.mouseCursor.coords.x << "," << event.mouseCursor.coords.y << ")" << std::endl;
          s.updateMouseCoords(event.mouseCursor.coords);
        }

        if(event.type == gf::EventType::MouseButtonPressed && !waitForAnswer) {

          // Clic gauche : choisir une pièce
          if(event.mouseButton.button == gf::MouseButton::Left) {
            // Récupération de la pièce dans le selecteur
            gf::Vector2i c = s.getPieceCoordsFromMouse(event.mouseButton.coords);
            // std::cout << "Case : ( " << c.x << " , " << c.y << " )" << std::endl;
            if(c.x != -1 && c.y != -1) {
              Piece p = s.getPiece({(unsigned)c.x, (unsigned)c.y});
              std::cout << "Piece : " << static_cast<int>(p.rank) << std::endl;
              // On sélectionne la pièce voulue
              s.selectPiece((unsigned int)p.rank);
            }

            // Récupération de la case de la grille
            c = g.getPieceCoordsFromMouse(event.mouseButton.coords);
            if(c.x != -1 && c.y != -1) {
              // Vérification qu'une pièce est selectionné dans le sélecteur
              if(s.selected != -1) {
                // On peut alors regarder la case cliquée
                Piece p = g.getPiece({(unsigned)c.x, (unsigned)c.y});
                std::cout << "Piece : " << static_cast<int>(p.rank) << std::endl;

                Piece newPiece;
                newPiece.rank = Rank(s.selected);
                newPiece.side = Side::Red;

                if(g.setPiece(c, newPiece)) {
                  s.takeOnePiece(s.selected);
                  s.selected = -1;
                } else {
                  std::cout << "Erreur" << std::endl;
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
            gf::Vector2i c = g.getPieceCoordsFromMouse(event.mouseButton.coords);
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

        ui.processEvent(event);
      }

      if(closeWindowAction.isActive()) {
        // Envoi d'un message de déconnexion au serveur
        Packet p;
        p.append(6); // Le client quitte
        send_packet(p);
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

      if(placementAleatoireAction.isActive()) {
        // Placement aléatoire de toutes les pièces restantes
        
        Piece p = s.getRandomPiece();
        while(p.rank != Rank::Empty) {
          std::cout << "Une pièce en plus !" << std::endl;
          g.setPieceRandom(p);
          s.takeOnePiece((int)p.rank);
          
          p = s.getRandomPiece();
        }
      }

      if(validAction.isActive()) {
        // On appuie sur entrer pour envoyer les pièces au serveur
        if(!s.isEmpty()) {
          // Il reste des pièces à placer
          errorNb = 1;
        } else {
          // Envoi des pièces au serveur
          Packet p;

          // Id du message
          p.append(1);

          // std::cout << "packet complet '";

          // Ajoute de toutes les pièces au packet
          for(int i = 0; i < 40; i++) {
            p.append(i); // Numéro de la pièces (à partir du bas à droite, vers le haut à gauche)
            // std::cout << i;
            p.append((char)(g.getPiece({g.GridSize - (i % g.GridSize) - 1, g.GridSize - (i / g.GridSize) - 1}).rank));
            // std::cout << (int)(g.getPiece({g.GridSize - (i % g.GridSize) - 1, g.GridSize - (i / g.GridSize) - 1}).rank);
            // std::cout << "x : " << g.GridSize - (i % g.GridSize) - 1 << " ; y : " << g.GridSize - (i / g.GridSize) - 1 << std::endl;
            // std::cout << (int)(g.getPiece({g.GridSize - (i % g.GridSize) - 1, g.GridSize - (i / g.GridSize) - 1}).rank) << std::endl;
          }

          // std::cout << "'" << std::endl;

          send_packet(p);

          // On attend maintenant la réponse :
          waitForAnswer = true;
        }
      }

      // Update
      gf::Time time = clock.restart();
      g.update(time);
      // entities.update(time);

      // Draw
      renderer.clear();
      entities.render(renderer);

      // Réception des messages
      boost::array<char, 128> msg;
      bool msgLu = messages.poll(msg);

      if(msgLu) {
        std::cout << "Message " << (int)msg[0] << std::endl;
        switch(msg[0]) { // Type du message
          case -1:
            // Erreur provenant du serveur
            fatalError = true;
            break;
          case 0: // Acceptation du serveur
            if(!waitForAnswer) {
              if(!msg[1]) {
                errorNb = 2;
              }
            } else {
              if(!msg[1]) {
                waitForAnswer = false;
                errorNb = 3;
              } else {
                setupFinished = true;
              }
            }
            break;
          default:
            break;
        }
      }

      // UI
      if(fatalError) {
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
        // Afficher la fenêtre d'UI
        if(ui.begin("Menu", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Minimizable | gf::UIWindow::Title)) {

          ui.layoutRowDynamic(25, 1);

          if(ui.buttonLabel("Quitter")) {
            // Envoi d'un message de déconnexion au serveur
            Packet p;
            p.append(6); // Le client quitte
            send_packet(p);
            window.close();
          }
        }

        ui.end();

        renderer.draw(ui);
      }

      if(errorNb != -1) {
        if(ui.begin("Erreur", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Minimizable | gf::UIWindow::Title)) {

          ui.layoutRowDynamic(25, 1);

          switch(errorNb) {
            case 1:
              ui.label("Il reste des pièces à placer");
              break;
            case 2:
              ui.label("Vous avez été refusé par le serveur");
              break;
            case 3:
              ui.label("Le placement des pièces est incorrect");
              break;
            default:
              ui.label("Une erreur est survenue");
              break;
          }

          if(ui.buttonLabel("OK")) {
            errorNb = -1;
          }

          ui.end();

          renderer.draw(ui);
        }
      }

      renderer.display();
  }

  std::cout << "Salut !" << std::endl;

  State state = State::WaitPlayer;

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

        if(event.type == gf::EventType::MouseButtonPressed) {

          if(event.mouseButton.button == gf::MouseButton::Left) {
            //if(!waitForAnswer && !pause) {
              // g.getPieceCoordsFromMouse(event.mouseCursor.coords);
              if(state != State::Play) {
                continue;
              }

              std::cout << "clic !" << std::endl;
              gf::Vector2i coords = g.getPieceCoordsFromMouse(event.mouseButton.coords);
              if(coords.x != -1 && coords.y != -1) {
                if(g.isSelected()) {
                  // if(g.moveSelectedPieceTo(coords)) {
                    // Envoi au serveur du mouvement
                    Packet p;
                    p.append(3);
                    // g.getPiece({g.GridSize - (i % g.GridSize) - 1, g.GridSize - (i / g.GridSize) - 1}
                    p.append((g.GridSize - g.selected.y - 1) * g.GridSize + (g.GridSize - g.selected.x - 1));
                    p.append((g.GridSize - coords.y - 1) * g.GridSize + (g.GridSize - coords.x - 1));
                    send_packet(p);
                    
                    state = State::WaitUpdateAnswer;
                  // }
                } else {
                  g.selectPiece({(unsigned)coords.x, (unsigned)coords.y});
                }
              }
            //}
          }
        }
      }

      if(closeWindowAction.isActive() && state != State::FatalError) {
        // Envoi d'un message de déconnexion au serveur
        Packet p;
        p.append(6); // Le client quitte
        send_packet(p);
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
      // entities.update(time);

      // Draw
      renderer.clear();
      entities.render(renderer);

      // Réception des messages
      boost::array<char, 128> msg;
      bool msgLu = messages.poll(msg);

      if(msgLu && state != State::FatalError) {
        std::cout << "Message lu : " << (int)msg[0] << std::endl;
        switch(msg[0]) { // Type du message
          case -1:
            // Erreur provenant du serveur
            state = State::FatalError;
            break;
          case 0: // Acceptation du serveur
            if(!msg[1]) {
              // Le mouvement n'a pas été accepté
              state = State::FatalError;
            } else if(state == State::WaitAnswer) {
              state = State::WaitUpdate;
            } else if(state == State::WaitUpdateAnswer) {
              state = State::WaitUpdateAfterAnswer;
            }
            break;
          case 2: // Jouer
            if(state == State::WaitPlayer) {
              state = State::WaitUpdate;
            } else {
              state = State::Play;
            }
            break;
          case 4: // Update
            if(state != State::WaitUpdate && state != State::WaitUpdateAfterAnswer) {
              state = State::FatalError;
              break;
            }

            Piece firstPiece;
            gf::Vector2u firstCoords;

            firstCoords.x = msg[1] % g.GridSize;
            firstCoords.y = (int)(msg[1] / g.GridSize);
            if(state == State::WaitUpdateAfterAnswer) {
              firstCoords.x = g.GridSize - 1 - firstCoords.x;
              firstCoords.y = g.GridSize - 1 - firstCoords.y;
            }
            std::cout << "Valeur : " << (int)msg[2] << std::endl;
            firstPiece.rank = (Rank)msg[2];
            firstPiece.side = state == State::WaitUpdate ? Side::Blue : Side::Red;

            Piece lastPiece;
            gf::Vector2u lastCoords;
            lastCoords.x = msg[3] % g.GridSize;
            lastCoords.y = (int)(msg[3] / g.GridSize);
            if(state == State::WaitUpdateAfterAnswer) {
              lastCoords.x = g.GridSize - 1 - lastCoords.x;
              lastCoords.y = g.GridSize - 1 - lastCoords.y;
            }
            std::cout << "Coords (first) : " << firstCoords.x << " ; " << firstCoords.y << std::endl;
            std::cout << "Coords (last) : " << lastCoords.x << " ; " << lastCoords.y << std::endl;
            lastPiece.rank = (Rank)msg[4];
            if(lastPiece.rank == Rank::Empty) {
              // Il y a eu égalité donc la case est vide
              lastPiece.side = Side::Other;
            } else if(lastPiece.rank == firstPiece.rank) {
              // Le rang de la pièce est inchangé
              lastPiece.side = state == State::WaitUpdate ? Side::Blue : Side::Red;
            } else {
              lastPiece.side = state == State::WaitUpdate ? Side::Red : Side::Blue;
            }

            state = State::WaitPlayer;

            if(!g.makeUpdate(firstCoords, firstPiece, lastCoords, lastPiece)) {
              state = State::FatalError; // Une erreur est survenue lors de l'update
            }
            break;
          default:
            break;
        }
      }

      // UI
      if(state == State::WaitUpdate || state == State::WaitPlayer) {
        if(ui.begin("Pause", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Title)) {

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
        if(ui.begin("Erreur", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Title)) {

          ui.layoutRowDynamic(25, 1);

          ui.label("Le serveur a rencontré une erreur");

          if(ui.buttonLabel("OK (quitter)")) {
            window.close();
          }
        }

        ui.end();
        renderer.draw(ui);
      } else if(displayEscUi) { // Menu ESC
        // Afficher la fenêtre d'UI
        if(ui.begin("Menu", gf::RectF(renderer.getSize().x / 2 - 100, renderer.getSize().y / 2 - 100, 200, 200), gf::UIWindow::Border | gf::UIWindow::Minimizable | gf::UIWindow::Title)) {

          ui.layoutRowDynamic(25, 1);

          if(ui.buttonLabel("Quitter")) {
            // Envoi d'un message de déconnexion au serveur
            Packet p;
            p.append(6); // Le client quitte
            send_packet(p);
            window.close();
          }
        }

        ui.end();
        renderer.draw(ui);
      }

      renderer.display();
  }

  return 0;
}
