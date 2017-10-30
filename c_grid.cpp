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
}

void Grid::createGrid() {
  m_layer.clear();

  for(unsigned x = 0; x < GridSize; x++) {
    for(unsigned y = 0; y < GridSize; y++) {
      gf::Vector2u coords(x, y);

      switch(grid[x][y].rank) {
        case Rank::Bomb:
          m_layer.setTile(coords, 0);
          break;
        default:
          m_layer.setTile(coords, 14);
          break;
      }
    }
  }
}

void Grid::render(gf::RenderTarget& target, const gf::RenderStates& states) {
  target.draw(m_layer);
}
