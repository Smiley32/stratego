#include "c_grid.h"

#include <iostream>

Grid::Grid(gf::ResourceManager& resources)
: m_layer({GridSize, GridSize})
{
  m_layer.setTileSize({TileSize, TileSize});
  m_layer.setTexture(resources.getTexture("pieces.png"));

  // Initialisation de la grille
  for(unsigned x = 0; x < GridSize; x++) {
    for(unsigned y = 0; y < GridSize; y++) {
      grid[x][y].side = Side::Other;
      grid[x][y].rank = Rank::Empty;
    }
  }

  grid[2][4].rank = Rank::Water;
  grid[3][4].rank = Rank::Water;
  grid[2][5].rank = Rank::Water;
  grid[3][5].rank = Rank::Water;

  grid[6][4].rank = Rank::Water;
  grid[6][5].rank = Rank::Water;
  grid[7][4].rank = Rank::Water;
  grid[7][5].rank = Rank::Water;
}

void Grid::createGrid() {
  m_layer.clear();

  for(unsigned x = 0; x < GridSize; x++) {
    for(unsigned y = 0; y < GridSize; y++) {
      gf::Vector2u coords(x, y);
      m_layer.setTile(coords, static_cast<int>(grid[x][y].rank));
    }
  }
}

void Grid::setPosition(gf::Vector2f origin) {
  m_layer.setPosition(origin);
}

gf::Vector2f Grid::getPosition() {
  return m_layer.getPosition();
}

gf::Vector2i Grid::getPieceCoordsFromMouse(gf::Vector2f coords) {
  if(coords.x < getPosition().x || coords.y < getPosition().y || coords.x > getPosition().x + (GridSize * TileSize) || coords.y > getPosition().y + (GridSize * TileSize)) {
    return {-1, -1};
  }

  return {(int)((coords.x - getPosition().x) / TileSize), (int)((coords.y - getPosition().y) / TileSize)};
}

Piece Grid::getPiece(gf::Vector2u coords) {
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

void Grid::removePiece(gf::Vector2u coords) {
  grid[coords.x][coords.y].side = Side::Other;
  grid[coords.x][coords.y].rank = Rank::Empty;
}

bool Grid::setPiece(gf::Vector2u coords, Piece p) {
  if(grid[coords.x][coords.y].rank == Rank::Empty && coords.y >= 6) {
    grid[coords.x][coords.y].rank = p.rank;
    grid[coords.x][coords.y].side = p.side;
    modif = true;
    std::cout << "..." << coords.x << "," << coords.y << " ; " << (int)grid[coords.x][coords.y].rank << std::endl;
    return true;
  }
  return false;
}

void Grid::render(gf::RenderTarget& target, const gf::RenderStates& states) {
  // std::cout << "Dessin..." << std::endl;
  // target.draw(m_layer, states);

  gf::Texture texture;
  texture.loadFromFile("pieces.png");

  for(unsigned x = 0; x < GridSize; x++) {
    for(unsigned y = 0; y < GridSize; y++) {
      gf::Vector2u coords(x, 0);
      int r = (int)grid[x][y].rank;

      gf::Sprite sprite(texture, gf::RectF( ((r * TileSize) % 256) / 256.0, (((r * TileSize) / 256) * TileSize) / 256.0, TileSize / 256.0, TileSize / 256.0));
      sprite.setPosition({getPosition().x + (x * TileSize), getPosition().y + (y * TileSize)});
      target.draw(sprite, states);
    }
  }
}

void Grid::update(gf::Time time) {
  if(modif) {
    std::cout << "update" << std::endl;
    modif = false;

    m_layer.clear();
    for(unsigned x = 0; x < GridSize; x++) {
      for(unsigned y = 0; y < GridSize; y++) {
        gf::Vector2u coords(x, y);
        m_layer.setTile(coords, static_cast<int>(grid[x][y].rank));
        // std::cout << "[" << coords.x << ", " << coords.y << "] - " << m_layer.getTile(coords) << std::endl;
      }
    }
  }
}
