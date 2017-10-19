#ifndef C_PIECE_H
#define C_PIECE_H

#include <gf/RenderWindow.h>
#include <gf/Sprite.h>

class Piece {
public:
  void setPosition(gf::Vector2i pos);
  void render(gf::RenderTarget& target);
  void setSprite(gf::Sprite& sprite);
  int getValue();
private:
  int width, height; // Taille de l'image
  gf::Sprite sprite;
  int value;
};

#endif
