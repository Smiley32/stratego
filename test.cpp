#include <gf/Clock.h>
#include <gf/Color.h>
#include <gf/Event.h>
#include <gf/RenderWindow.h>
#include <gf/Window.h>

int main() {
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
  }
  
  return 0;
}
