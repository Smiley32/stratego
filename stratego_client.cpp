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

#include <gf/Sleep.h>
#include <gf/Time.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <string>

#include <thread>
#include <sstream>

#include "c_grid.h"

using boost::asio::ip::tcp;

// Récupération d'un message du serveur
std::string get_message(tcp::socket *socket) {
  boost::array<char, 128> buf;
  boost::system::error_code error;

  size_t len = socket->read_some(boost::asio::buffer(buf), error);

  if(error) {
    throw boost::system::system_error(error);
  }

  std::string data(buf.begin(), buf.begin() + len);
  return data;
}

// Thread qui va communiquer avec le serveur (le parmaètre est juste un test)
void reception_thread(std::string msg) {
  // Affichage du paramètre
  std::cout << msg << std::endl;

  try {
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);

    std::string ip;
    std::cout << "Entrez l'ip du serveur :" << std::endl;
    std::cin >> ip;

    std::string port;
    std::cout << "Entrez le port du serveur :" << std::endl;
    std::cin >> port;

    tcp::resolver::query query(ip, port);

    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    // Création et connection du socket
    tcp::socket socket(io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;

    while(error && endpoint_iterator != end) {
      socket.close();
      socket.connect(*endpoint_iterator++, error);
    }

    if(error) {
      throw boost::system::system_error(error);
    }

    // La connection est ouverte
    std::cout << get_message(&socket) << std::endl;

  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}

int main(int argc, char *argv[]) {

  // std::thread rt(reception_thread, "Bonjour, je suis le thread de réception !");
  // rt.join();

  // Initialisation
  static constexpr gf::Vector2u ScreenSize(640, 640);
  gf::Window window("Petit jeu en réseau (client)", ScreenSize);
  window.setFramerateLimit(60);
  gf::RenderWindow renderer(window);

  // Actions
  gf::ActionContainer actions;

  gf::Action closeWindowAction("Close window");
  closeWindowAction.addCloseControl();
  closeWindowAction.addKeycodeKeyControl(gf::Keycode::Escape);
  actions.addAction(closeWindowAction);

  // Resource manager
  gf::ResourceManager resources;
  resources.addSearchDir(".");

  gf::EntityContainer entities;

  Grid g(resources);
  g.createGrid();
  entities.addEntity(g);

  // Boucle de jeu
  renderer.clear(gf::Color::White);

  gf::Clock clock;

  while(window.isOpen()) {
      gf::Event event;

      // Entrées
      while(window.pollEvent(event)) {
        actions.processEvent(event);

        if(event.type == gf::EventType::MouseMoved) {
          std::cout << "Case : ( " << event.mouseCursor.coords.x / g.TileSize << " , " << event.mouseCursor.coords.y / g.TileSize << " )" << std::endl;
          Piece p = g.getPiece({event.mouseCursor.coords.x / g.TileSize, event.mouseCursor.coords.y / g.TileSize});

          std::cout << "Piece : " << static_cast<int>(p.rank) << std::endl;
        }
      }

      if(closeWindowAction.isActive()) {
        window.close();
      }

      // Update
      gf::Time time = clock.restart();

      // Draw
      renderer.clear();
      entities.render(renderer);

      // Récupération de la dernière coordonnée de la file
      renderer.display();
  }

  return 0;
}
