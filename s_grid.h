#ifndef SGRID_H
#define SGRID_H

#include "s_piece.h"

class s_grid
{

  public:

    void create_empty_grid();
    bool start_game();

  private:

    Piece grid[10][10];
    size_t size;
    bool is_started;

    bool create_piece(gf::Vector2u coo2D, Piece p);
};

#endif
