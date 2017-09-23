#include <gf/Clock.h>
#include <gf/Color.h>
#include <gf/Event.h>
#include <gf/RenderWindow.h>
#include <gf/Window.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <iostream>

using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {
  
  // Test de client
  try {
    // Le client doit spécifier le serveur en argument
    if(argc != 2) {
      std::cerr << "Usage : <host>" << std::endl;
      return 1;
    }
    
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1], "25565");
    
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
    for(;;) {
      boost::array<char, 128> buf;
      boost::system::error_code error;
      
      size_t len = socket.read_some(boost::asio::buffer(buf), error);
      
      if(error == boost::asio::error::eof) {
        // Connection fermée
        break;
      }
      
      if(error) {
        throw boost::system::system_error(error);
      }
      
      // Affichage du message
      std::cout.write(buf.data(), len);
    }
    
  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  
  /*
  // Initialisation
  gf::Window window("Ma première fenêtre !", {1280, 720});
  gf::RenderWindow renderer(window);
  
  // Boucle de jeu
  gf::Clock clock;
  renderer.clear(gf::Color::White);
  
  while(window.isOpen()) {
    gf::Event event;
    
    // Entrées
    while(window.pollEvent(event)) {
      switch(event.type) {
        case gf::EventType::Closed:
          window.close();
          break;
        
        case gf::EventType::KeyPressed:
          
          break;
        
        default:
          break;
      }
    }
    
    // Mise à jour de l'affichage
    float dt = clock.restart().asSeconds();
    
    renderer.clear();
    renderer.display();
  }*/
  
  return 0;
}
