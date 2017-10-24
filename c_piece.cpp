#include "c_piece.h"
#include <gf/VectorOps.h>
#include <gf/RenderTarget.h>

Piece::Piece(gf::ResourceManager& resources)
, p_texture(resources.getTexture("pieces.png"))
{
  p_texture.setSmooth();
}

void Piece::draw(gf::RenderTarget& target, gf::Vector2i pos, int piece) {
  gf::Vector2f TileScale(pieceWidth / tilesetWidth, pieceHeight / tilesetHeight);
  gf::Sprite sprite(p_texture, gf::RectF(TileScale * gf::Vector2u(1, 1), TileScale));
  sprite.setAnchor(gf::Anchor::Center);
  sprite.setPosition(pos);

  target.draw(sprite);
}
