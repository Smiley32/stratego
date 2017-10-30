#ifndef C_GRID_H
#define C_GRID_H

#include <gf/Entity.h>
#include <gf/TileLayer.h>
#include "c_piece.h"

class Grid : public gf::Entity {
public:
  static constexpr unsigned TileSize = 64;

  Grid(gf::ResourceManager& resources);
  void createGrid();

  virtual void render(gf::RenderTarget& target, const gf::RenderStates& states) override;

  Piece getPiece(gf::Vector2u coords);

private:
  static constexpr unsigned GridSize = 10;
  gf::TileLayer m_layer;

  Piece grid[GridSize][GridSize];
};

#endif
