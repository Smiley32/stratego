#ifndef SGRID_H
#define SGRID_H

#include "s_piece.h"
#include <gf/Entity.h>
#include <gf/TileLayer.h>
#include <gf/RenderTarget.h>
#include <gf/Curves.h>
#include <gf/Color.h>

class s_grid
{

  public:

    void create_empty_grid();
    bool start_game();
    bool red_t_ok();
    bool blue_t_ok();
    bool create_piece(gf::Vector2u coo2D, Piece p);
    bool move_piece(gf::Vector2u source, gf::Vector2u dest);

  private:

    Piece grid[10][10];
    size_t size;
    bool is_started;
    int b_pieces[12];
    int r_pieces[12];
};

#endif
