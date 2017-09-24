#include <gf/Clock.h>
#include <gf/Color.h>
#include <gf/Event.h>
#include <gf/RenderWindow.h>
#include <gf/Shapes.h>
#include <gf/Vector.h>
#include <gf/Window.h>
#include <gf/Queue.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <string>

#include <thread>

using boost::asio::ip::tcp;

gf::Queue<gf::Vector2f> file;

void reception_thread(std::string msg) {
  return;
  std::cout << msg << std::endl;
  // Dans ce thread, on communique avec le serveur

  try {
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query("192.168.1.18", "25565");
    
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
      
      // On envoie un message au serveur
      boost::system::error_code ignored_error;
      std::string s("Je suis connecté !");
      boost::asio::write(socket, boost::asio::buffer(s), boost::asio::transfer_all(), ignored_error);
      
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
      
      break;
    }
    
  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}

class Square {
  public:
    Square(gf::Vector2f position, float size, gf::Color4f color)
    : m_position(position)
    , m_velocity(0, 0)
    , m_size(size)
    , m_color(color)
    {}
    
    void setVelocity(gf::Vector2f velocity) {
      m_velocity = velocity;
    }
    
    void update(float dt) {
      m_position += dt * m_velocity;
    }
    
    gf::Vector2f getPosition() {
      return m_position;
    }
    
    void render(gf::RenderTarget& target) {
      gf::RectangleShape shape({m_size, m_size});
      shape.setPosition(m_position);
      shape.setColor(m_color);
      shape.setAnchor(gf::Anchor::Center);
      target.draw(shape);
    }
    
  private:
    gf::Vector2f m_position;
    gf::Vector2f m_velocity;
    float m_size;
    gf::Color4f m_color;
};

void send_pos(tcp::socket *socket, gf::Vector2f pos) {
  boost::system::error_code ignored_error;
  std::string s;
  s = std::to_string(pos.x) + ";" + std::to_string(pos.y);
  boost::asio::write(*socket, boost::asio::buffer(s), boost::asio::transfer_all(), ignored_error);
}

int main(int argc, char *argv[]) {
  
  try {
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query("192.168.1.18", "25565");
    
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
    
    
    /*std::thread rt(reception_thread, "Bonjour, je suis le thread de réception !");
    rt.join();*/
    
    // Initialisation
    static constexpr gf::Vector2u ScreenSize(500, 500);
    gf::Window window("Petit jeu en réseau (client)", ScreenSize);
    gf::RenderWindow renderer(window);
    
    // Entités
    Square entity(ScreenSize / 2, 50.0f, gf::Color::Red);
    
    // Boucle de jeu
    gf::Clock clock;
    renderer.clear(gf::Color::White);
    
    static constexpr float Speed = 100.0f;
    gf::Vector2f velocity(0, 0);
    
    while(window.isOpen()) {
      gf::Event event;
      
      // Entrées
      while(window.pollEvent(event)) {
        switch(event.type) {
          case gf::EventType::Closed:
            window.close();
            break;
          
          case gf::EventType::KeyPressed:
            switch(event.key.keycode) {
              case gf::Keycode::Up:
                velocity.y -= Speed;
                break;
              case gf::Keycode::Down:
                velocity.y += Speed;
                break;
              case gf::Keycode::Left:
                velocity.x -= Speed;
                break;
              case gf::Keycode::Right:
                velocity.x += Speed;
                break;
              default:
                break;
            }
            break;
          
          case gf::EventType::KeyReleased:
            switch(event.key.keycode) {
              case gf::Keycode::Up:
                velocity.y += Speed;
                break;
              case gf::Keycode::Down:
                velocity.y -= Speed;
                break;
              case gf::Keycode::Left:
                velocity.x += Speed;
                break;
              case gf::Keycode::Right:
                velocity.x -= Speed;
                break;
              default:
                break;
            }
            break;
            
          default:
            break;
        }
      }
      
      // Mise à jour de l'affichage
      entity.setVelocity(velocity);
      float dt = clock.restart().asSeconds();
      entity.update(dt);
      send_pos(&socket, entity.getPosition());
      
      renderer.clear();
      entity.render(renderer);
      renderer.display();
    }
  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  
  return 0;
}
