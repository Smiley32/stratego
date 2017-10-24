#ifndef C_PIECE_H
#define C_PIECE_H

#include <gf/RenderWindow.h>
#include <gf/Sprite.h>
#include <gf/ResourceManager.h>
#include <gf/Texture.h>

class Piece {
public:
  Piece(gf::ResourceManager& resources);

  /**
   * Dessiner la pièce à une position
   *
   * @param gf::RenderTarget& target  Renderer à utiliser
   * @param gf::Vector2i pos          Position de la pièce (sur l'écran)
   * @param int piece                 Le type de pièce à dessiner
   */
  void draw(gf::RenderTarget& target, gf::Vector2i pos, int piece);
private:
  // Taille des pièces : à modifier en fonction des sprites
  int pieceWidth = 42;
  int pieceHeight = 42;

  // Taille de l'image contenant les sprites
  int tilesetWidth = 420;
  int tilesetHeight = 420;

  // Texture contenant les sprites
  gf::Texture p_texture;
};

#endif
