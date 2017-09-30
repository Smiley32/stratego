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

// Objet Square : le carré qu'on affiche
class Square {
  public:
    // Constructeur
    Square(gf::Vector2f position, float size, gf::Color4f color)
    : m_position(position)
    , m_velocity(0, 0)
    , m_size(size)
    , m_color(color)
    {}

    // Définir la vélocité du carré
    void setVelocity(gf::Vector2f velocity) {
      m_velocity = velocity;
    }

    // Mise à jour de la position du carré en fonction du temps écoulé
    void update(float dt) {
      m_position += dt * m_velocity;
    }

    // Retourne la position du carré
    gf::Vector2f getPosition() {
      return m_position;
    }

    // Rendu du carré à l'écran
    void render(gf::RenderTarget& target) {
      // Taille du carré
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

// Envoi de la position du carré au socket socket
// Fonction aurait été mieux dans l'objet carré je pense
void send_pos(tcp::socket *socket, gf::Vector2f pos) {
  // Code d'erreur (qu'on ignore ici)
  boost::system::error_code ignored_error;
  std::string s = std::to_string(pos.x) + ";" + std::to_string(pos.y);
  boost::asio::write(*socket, boost::asio::buffer(s), boost::asio::transfer_all(), ignored_error);
}

int main(int argc, char *argv[]) {
  try {
    boost::asio::io_service io_service;

    // Connexion au serveur
    tcp::resolver resolver(io_service);
    // ip et port (port peut être remplacé par le nom du processus)
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

    // En cas d'erreur
    if(error) {
      throw boost::system::system_error(error);
    }

    // La connexion est effectuée avec le serveur via le socket "socket"

    // Initialisation de la fenêtre
    static constexpr gf::Vector2u ScreenSize(500, 500);
    gf::Window window("Petit jeu en réseau (client)", ScreenSize);
    // Création d'un renderer (pour faire les affichages)
    gf::RenderWindow renderer(window);

    // Entités (ici seulement le carré, placé au centre de l'écran)
    Square entity(ScreenSize / 2, 50.0f, gf::Color::Red);

    // Boucle de jeu
    gf::Clock clock;
    // Effacage de l'écran
    renderer.clear(gf::Color::White);

    // Vitesse du carré
    static constexpr float Speed = 100.0f;
    gf::Vector2f velocity(0, 0);

    // Boucle de jeu
    while(window.isOpen()) {
      gf::Event event;

      // Entrées (clavier, souris, etc.)
      // Boucle des événements
      while(window.pollEvent(event)) {
        switch(event.type) {
          // Si on a demandé à fermer la fenêtre
          case gf::EventType::Closed:
            window.close();
            break;

          // Une touche du clavier pressée
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

          // Une touche du clavier relachée
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

          // Autres cas
          default:
            break;
        }
      }

      // Mise à jour de l'affichage
      entity.setVelocity(velocity);
      float dt = clock.restart().asSeconds();
      entity.update(dt);

      // Envoi de la position au serveur
      send_pos(&socket, entity.getPosition());

      // Effacage puis réaffichage de l'écran
      renderer.clear();
      entity.render(renderer);
      renderer.display();
    }
  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
