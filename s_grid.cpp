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
}


bool create_piece(gf::Vector2u coo2D, Piece p)
{
  // Vérification jeu non lancé
  if (is_started)
  {
    fprintf(stderr, "Error create_piece: game already started\n");
    return false;
  }

  if (coo2D.x > size || cooD.y > size || coo2D.x < 0 || coo2D.y < 0)
  {
    fprintf(stderr, "Error create_piece: Invalid coords %d %d\n", coo2D.x, coo2D.y);
    return false;
  }

  // Vérification pièce du bon côté
  if ( (p.side == Side::Blue && coo2D.y < 6) || (p.side == Side::Red && coo2D.y > 3) )
  {
    fprintf(stderr, "Error create_piece: Wrong side for Piece %d (%s Team): Tried %d %d\n", p.rank, p.side, coo2D.x, Coo2D.y);
    return false;
  }

  // Vérication case libre
  if (grid[coo2D.x][coo2D.y].rank != Rank::Empty)
  {
    fprintf(stderr, "Error create_piece: %d %d is not free", coo2D.x, coo2D.y);
    return false;
  }

  grid[coo2D.x][coo2D.y].rank = p.rank;
  grid[coo2D.x][coo2D.y].side = p.side;

  return true;
}

bool start_game()
{
  int blue_team[12];
  int red_team[12];

  blue_team[0] = red_team[0] = 6;
  blue_team[1] = red_team[1] = 1;
  blue_team[2] = red_team[2] = 1;
  blue_team[3] = red_team[3] = 2;
  blue_team[4] = red_team[4] = 3;
  blue_team[5] = red_team[5] = 4;
  blue_team[6] = red_team[6] = 4;
  blue_team[7] = red_team[7] = 4;
  blue_team[8] = red_team[8] = 5;
  blue_team[9] = red_team[9] = 8;
  blue_team[10] = red_team[10] = 1;
  blue_team[11] = red_team[11] = 1;

  // Comptage côté rouge
  for (size_t x = 0; x < size; x++)
  {
    for (size_t y = 0; y < 4; y++)
    {
      if (grid[x][y].rank == Rank::Empty)
      {
        fprintf(stderr, "Error start_game: %zu %zu Empty", x, y);
        return false;
      }

      red_team[grid[x][y].rank]--;
    }
  }

  // Comptage côté bleu
  for (size_t x = 0; x < size; x++)
  {
    for (size_t y = 6; y < size; y++)
    {
      if (grid[x][y].rank == Rank::Empty)
      {
        fprintf(stderr, "Error start_game: %zu %zu Empty", x, y);
        return false;
      }

      blue_team[grid[x][y].rank]--;
    }
  }

  // On vérifie qu'il n'y ai pas d'anomalie dans le comptage de piece
  for (size_t i = 0; i < 12; i++)
  {
    if (red_team[i] != 0 || blue_team[i] != 0)
    {
      fprintf(stderr, "Error start_game: Wrong number of pieces %zu", i);
      return false;
    }
  }

  is_started = true;
  return is_started;
}

bool move_piece(gf::Vector2u source, gf::Vector2u dest)
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
  if (grid[source.x][source.y].rank > 10 || grid[source.x][source.y].rank == 0)
  {
    fprintf(stderr, "Error move_piece: This piece can't move\n");
    return false;
  }

  // TODO Colisions de pièces
}
