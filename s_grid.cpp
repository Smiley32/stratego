#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "s_grid.h"

using boost::asio::ip::tcp;

void s_grid::create_empty_grid()
{
  is_started = false;

  for(size_t x = 0; x < size; x++)
  {
    for(size_t y = 0; y < size; y++)
    {
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

  b_pieces[0] = r_pieces[0] = 6;
  b_pieces[1] = r_pieces[1] = 1;
  b_pieces[2] = r_pieces[2] = 1;
  b_pieces[3] = r_pieces[3] = 2;
  b_pieces[4] = r_pieces[4] = 3;
  b_pieces[5] = r_pieces[5] = 4;
  b_pieces[6] = r_pieces[6] = 4;
  b_pieces[7] = r_pieces[7] = 4;
  b_pieces[8] = r_pieces[8] = 5;
  b_pieces[9] = r_pieces[9] = 8;
  b_pieces[10] = r_pieces[10] = 1;
  b_pieces[11] = r_pieces[11] = 1;
}


bool s_grid::create_piece(gf::Vector2u coo2D, Piece p)
{
  // Vérification jeu non lancé
  if (is_started)
  {
    fprintf(stderr, "Error create_piece: game already started\n");
    return false;
  }

  if (coo2D.x > size || coo2D.y > size || coo2D.x < 0 || coo2D.y < 0)
  {
    fprintf(stderr, "Error create_piece: Invalid coords %d %d\n", coo2D.x, coo2D.y);
    return false;
  }

  // Vérification pièce du bon côté
  if ( (p.side == Side::Blue && coo2D.y < 6) || (p.side == Side::Red && coo2D.y > 3) )
  {
    fprintf(stderr, "Error create_piece: Wrong side for Piece %d (%s Team): Tried %d %d\n", (int) p.rank, (char *) p.side, coo2D.x, coo2D.y);
    return false;
  }

  // Vérication case libre
  if (grid[coo2D.x][coo2D.y].rank != Rank::Empty)
  {
    fprintf(stderr, "Error create_piece: %d %d is not free", coo2D.x, coo2D.y);
    return false;
  }

  if (p.side == Side::Blue)
  {
    if (b_pieces[(int) p.rank] <= 0)
    {
      return false;
    }
    b_pieces[(int) p.rank]--;
  }
  else
  {
    if (r_pieces[(int) p.rank] <= 0)
    {
      return false;
    }
    r_pieces[(int) p.rank]--;
  }
  grid[coo2D.x][coo2D.y].rank = p.rank;
  grid[coo2D.x][coo2D.y].side = p.side;

  return true;
}

bool s_grid::start_game()
{

  // On vérifie qu'il n'y ai pas d'anomalie dans le comptage de piece
  for (size_t i = 0; i < 12; i++)
  {
    if (r_pieces[i] != 0 || b_pieces[i] != 0)
    {
      fprintf(stderr, "Error start_game: Wrong number of pieces %zu", i);
      return false;
    }
  }

  is_started = true;
  return is_started;
}

bool s_grid::red_t_ok()
{
  for (size_t i = 0; i < 12; i++)
  {
    if (r_pieces[i] != 0)
    {
      fprintf(stderr, "Error start_game: Wrong number of pieces %zu", i);
      return false;
    }
  }

  return true;
}

bool s_grid::blue_t_ok()
{
  for (size_t i = 0; i < 12; i++)
  {
    if (b_pieces[i] != 0)
    {
      fprintf(stderr, "Error start_game: Wrong number of pieces %zu", i);
      return false;
    }
  }

  return true;
}

bool s_grid::move_piece(gf::Vector2u source, gf::Vector2u dest)
{
  // Vérifications coordonnées dans la carte
  if (source.x < 0 || source.y < 0 || source.x > size || source.y > size)
  {
    fprintf(stderr, "Error move_piece: Invalid coord for source %d %d\n", source.x, source.y);
    return false;
  }

  if (dest.x < 0 || dest.y < 0 || dest.x > size || dest.y > size)
  {
    fprintf(stderr, "Error move_piece: Invalid coord for dest %d %d\n", dest.x, dest.y);
    return false;
  }

  // Vérification piece a le droit de bouger
  if ((int) grid[source.x][source.y].rank > 10 || (int) grid[source.x][source.y].rank == 0)
  {
    fprintf(stderr, "Error move_piece: This piece can't move\n");
    return false;
  }

  // TODO Colisions de pièces
}
