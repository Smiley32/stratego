#include "c_piece.h"

Selection::Selection(gf::ResourceManager& resources)
: m_layer({NbPieces, 1})
{
  m_layer.setTileSize({TileSize, TileSize});
  m_layer.setTexture(resources.getTexture("pieces.png"));

  for(unsigned x = 0; x < NbPieces; x++) {
    grid[x].side = Side::Red;
    grid[x].rank = (Rank)x;
  }

  m_layer.clear();
  for(unsigned x = 0; x < NbPieces; x++) {
    gf::Vector2u coords(x, 0);
    m_layer.setTile(coords, static_cast<int>(grid[x].rank));
  }
}

void Selection::setPosition(gf::Vector2f origin) {
  m_layer.setPosition(origin);
}

gf::Vector2f Selection::getPosition() {
  return m_layer.getPosition();
}

Piece Selection::getPiece(gf::Vector2u coords) {
  int tile = m_layer.getTile(coords);
  Piece p;
  if(tile >= 12) {
    p.side = Side::Other;
  } else {
    p.side = Side::Red;
  }
  p.rank = (Rank)tile;

  return p;
}

gf::Vector2i Selection::getPieceCoordsFromMouse(gf::Vector2f coords) {
  if(coords.x < getPosition().x || coords.y < getPosition().y || coords.x > getPosition().x + (NbPieces * TileSize) || coords.y > getPosition().y + TileSize) {
    return {-1, -1};
  }

  return {(int)((coords.x - getPosition().x) / TileSize), (int)((coords.y - getPosition().y) / TileSize)};
}

void Selection::selectPiece(unsigned int pieceNumber) {
  if(pieceNumber < NbPieces && nbPieces[pieceNumber] > 0) {
    selected = pieceNumber;
    grid[selected].rank = Rank::Empty;
  } else {
    selected = -1;
  }
}

void Selection::getPiece(unsigned int pieceNumber) {
  nbPieces[pieceNumber]--;
}

void Selection::render(gf::RenderTarget& target, const gf::RenderStates& states) {

  gf::Texture texture;
  texture.loadFromFile("pieces.png");

  for(unsigned x = 0; x < NbPieces; x++) {
    gf::Vector2u coords(x, 0);
    int r = (int)grid[x].rank;

    gf::Sprite sprite(texture, gf::RectF( ((r * TileSize) % 256) / 256.0, (((r * TileSize) / 256) * TileSize) / 256.0, TileSize / 256.0, TileSize / 256.0));
    sprite.setPosition({getPosition().x + (x * TileSize), getPosition().y});
    target.draw(sprite, states);
    // m_layer.setTile(coords, static_cast<int>(grid[x].rank));
  }

  // target.draw(m_layer);
}
