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
    int get_value(gf::Vector2u coo2D);
    Side get_side(gf::Vector2u coo2D);
    bool start_game();
    bool red_t_ok();
    bool blue_t_ok();
    bool game_is_end();
    bool had_collision();
    bool create_piece(gf::Vector2u coo2D, Piece p);
    bool move_piece(gf::Vector2u source, gf::Vector2u dest);

  private:

    Piece grid[10][10];
    size_t size = 10;
    bool is_started;
    bool collision;
    bool is_end;
    int b_pieces[12];
    int r_pieces[12];
};

#endif
