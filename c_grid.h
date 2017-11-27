#ifndef C_GRID_H
#define C_GRID_H

#include <gf/Entity.h>
#include <gf/TileLayer.h>
#include <gf/RenderTarget.h>
#include <gf/Curves.h>
#include <gf/Color.h>

#include "c_piece.h"

class Grid : public gf::Entity {
public:
  static constexpr unsigned TileSize = 64;
  static constexpr unsigned GridSize = 10;

  Grid(gf::ResourceManager& resources);
  void createGrid();

  void setPosition(gf::Vector2f origin);
  gf::Vector2f getPosition();

  gf::Vector2i getPieceCoordsFromMouse(gf::Vector2f coords);

  virtual void render(gf::RenderTarget& target, const gf::RenderStates& states) override;
  virtual void update(gf::Time time) override;

  Piece getPiece(gf::Vector2u coords);

  bool setPiece(gf::Vector2u coords, Piece p);

  void removePiece(gf::Vector2u coords);

private:
  gf::TileLayer m_layer;
  // Indique si une modification a eu lieu sur la grille
  bool modif = false;

  Piece grid[GridSize][GridSize];
};

#endif
