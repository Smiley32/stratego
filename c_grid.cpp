#include "c_grid.h"

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

  grid[3][8].side = Side::Blue;
  grid[3][8].rank = Rank::Bomb;
  grid[4][2].rank = Rank::Water;
  grid[4][3].rank = Rank::Water;
  grid[5][2].rank = Rank::Water;
  grid[5][3].rank = Rank::Water;

  grid[4][6].rank = Rank::Water;
  grid[4][7].rank = Rank::Water;
  grid[5][6].rank = Rank::Water;
  grid[5][7].rank = Rank::Water;
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

Piece Grid::getPiece(gf::Vector2u coords) {
  int tile = m_layer.getTile(coords);
  Piece p;
  switch(tile) {
    case 0:
      p.side = Side::Blue;
      p.rank = Rank::Bomb;
      break;
    case 1:
      p.side = Side::Blue;
      p.rank = Rank::Marshal;
      break;
    case 2:
      p.side = Side::Blue;
      p.rank = Rank::General;
      break;
    case 3:
      p.side = Side::Blue;
      p.rank = Rank::Colonel;
      break;
    case 4:
      p.side = Side::Blue;
      p.rank = Rank::Major;
      break;
    case 5:
      p.side = Side::Blue;
      p.rank = Rank::Captain;
      break;
    case 6:
      p.side = Side::Blue;
      p.rank = Rank::Lieutenant;
      break;
    case 7:
      p.side = Side::Blue;
      p.rank = Rank::Sergeant;
      break;
    case 8:
      p.side = Side::Blue;
      p.rank = Rank::Miner;
      break;
    case 9:
      p.side = Side::Blue;
      p.rank = Rank::Scout;
      break;
    case 10:
      p.side = Side::Blue;
      p.rank = Rank::Spy;
      break;
    case 11:
      p.side = Side::Blue;
      p.rank = Rank::Flag;
      break;
    case 12:
      p.side = Side::Other;
      p.rank = Rank::Water;
      break;
    default:
      p.side = Side::Other;
      p.rank = Rank::Empty;
      break;
  }

  return p;
}

void Grid::render(gf::RenderTarget& target, const gf::RenderStates& states) {
  target.draw(m_layer);
}
