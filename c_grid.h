#ifndef C_GRID_H
#define C_GRID_H

#include <gf/RenderWindow.h>
#include "c_piece.h"

class Grid {
public:
  Piece getPiece(gf::Vector2i pos);
private:
  int grid[10][10];
};

#endif
