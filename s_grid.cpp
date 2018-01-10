#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "s_grid.h"

using boost::asio::ip::tcp;

void s_grid::create_empty_grid()
{
  is_started = false;
  is_end = false;

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

  gf::Log::info("\n\tGrid created\n");
}

int s_grid::get_value(gf::Vector2u coo2D)
{
  return (int) grid[coo2D.x][coo2D.y].rank;
}

bool s_grid::create_piece(gf::Vector2u coo2D, Piece p)
{
  // Vérification jeu non lancé
  if (is_started)
  {
    gf::Log::fatal("\n\tError create_piece: game already started\n");
    return false;
  }

  if (coo2D.x > size || coo2D.y > size || coo2D.x < 0 || coo2D.y < 0)
  {
    gf::Log::error("\n\tError create_piece: Invalid coords %d %d\n", coo2D.x, coo2D.y);
    return false;
  }

  // Vérification pièce du bon côté
  if ( (p.side == Side::Blue && coo2D.y < 6) || (p.side == Side::Red && coo2D.y > 3) )
  {
    gf::Log::error("\n\tError create_piece: Wrong side for Piece %d (%s Team): Tried %d %d\n", (int) p.rank, (char *) p.side, coo2D.x, coo2D.y);
    return false;
  }

  // Vérication case libre
  if (grid[coo2D.x][coo2D.y].rank != Rank::Empty)
  {
    gf::Log::error("\n\tError create_piece: %d %d is not free, rank is %d\n", coo2D.x, coo2D.y, (int) p.rank);
    return false;
  }

  if (p.side == Side::Blue)
  {
    if (b_pieces[(int) p.rank] <= 0)
    {
      gf::Log::error("\n\tError create_piece: Too much piece of rank %d for %s Team", (int) p.rank, (char *) p.side);
      return false;
    }
    b_pieces[(int) p.rank]--;
  }
  else
  {
    if (r_pieces[(int) p.rank] <= 0)
    {
      gf::Log::error("\n\tError create_piece: Too much piece of rank %d for %s Team", (int) p.rank, (char *) p.side);
      return false;
    }
    r_pieces[(int) p.rank]--;
  }
  grid[coo2D.x][coo2D.y].rank = p.rank;
  grid[coo2D.x][coo2D.y].side = p.side;

  gf::Log::info("\n\tcreate_piece: Piece created in %d %d\n", coo2D.x, coo2D.y);
  return true;
}

bool s_grid::start_game()
{

  // On vérifie qu'il n'y ai pas d'anomalie dans le comptage de piece
  for (size_t i = 0; i < 12; i++)
  {
    if (r_pieces[i] != 0 || b_pieces[i] != 0)
    {
      gf::Log::error("\n\tError start_game: Wrong number of pieces %zu\n", i);
      return false;
    }
  }

  is_started = true;
  gf::Log::info("\n\tGame Started\n");
  return is_started;
}

bool s_grid::red_t_ok()
{
  for (size_t i = 0; i < 12; i++)
  {
    if (r_pieces[i] != 0)
    {
      gf::Log::error("\n\tError start_game: Wrong number of pieces %zu\n", i);
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
      gf::Log::error("\n\tError start_game: Wrong number of pieces %zu\n", i);
      return false;
    }
  }

  return true;
}

bool s_grid::move_piece(gf::Vector2u source, gf::Vector2u dest)
{
  Piece tmp;

  gf::Log::info("\n\tTry to move %d %d to %d %d\n", source.x, source.y, dest.x, dest.y);
  // Vérifications coordonnées dans la carte
  if (source.x < 0 || source.y < 0 || source.x > size || source.y > size)
  {
    gf::Log::error("\n\tError move_piece: Invalid coord for source %d %d\n", source.x, source.y);
    return false;
  }

  if (dest.x < 0 || dest.y < 0 || dest.x > size || dest.y > size)
  {
    gf::Log::error("\n\tError move_piece: Invalid coord for dest %d %d\n", dest.x, dest.y);
    return false;
  }

  // Vérification piece a le droit de bouger
  if ((int) grid[source.x][source.y].rank > 10 || (int) grid[source.x][source.y].rank == 0)
  {
    gf::Log::error("\n\tError move_piece: This piece can't move\n");
    return false;
  }

  // Si on a affaire à un éclaireur
  if ((int) grid[source.x][source.y].rank == 9)
  {
    // On vérifie juste que le déplacement est rectiligne
    if ( !(abs(source.x - dest.x) != 0 && abs(source.y - dest.y) == 0) && !(abs(source.x - dest.x) == 0 && abs(source.y - dest.y) != 0) )
    {
      gf::Log::error("\n\tError move_piece: Invalid deplacement, must be straight\n");
      return false;
    }

    // Vérification présence enemie à côté
    if (source.x != dest.x)
    { // Si on se déplace horizontalement
      for (size_t i = source.x + 1; i < dest.x; i++)
      {
        // On vérifie qu'il n'y a pas des pièces sur la trajectoire
        if ((int) grid[i][dest.y].rank != 13)
        {
          gf::Log::error("\n\tError move_piece: You can't jump over an other piece\n");
          return false;
        }

        // On vérifie la présence d'énnemis en haut
        if (source.y != 0)
        {
          if (grid[i][dest.y-1].side != grid[source.x][source.y].side && grid[i][dest.y-1].side != Side::Other)
          {
            gf::Log::error("\nError move_piece: You can't pass by enemies\n");
            return false;
          }
        }
        // On vérifie la présence d'ennemis en bas
        if (source.y != 9)
        {
          if (grid[i][dest.y+1].side != grid[source.x][source.y].side && grid[i][dest.y-1].side != Side::Other)
          {
            gf::Log::error("\nError move_piece: You can't pass by enemies\n");
            return false;
          }
        }
      }
    }
    else
    { // Si on se déplace verticalement
      for (size_t i = source.y + 1; i < dest.y; i++)
      {
        // On vérifie qu'il n'y a pas de pièces sur la trajectoire
        if ((int) grid[dest.x][i].rank != 13)
        {
          gf::Log::error("\n\tError move_piece: You can't jump over an other piece\n");
          return false;
        }

        // On vérifie la présence d'ennemis à gauche
        if (source.x != 0)
        {
          if (grid[dest.x-1][i].side != grid[source.x][source.y].side && grid[dest.x-1][i].side != Side::Other)
          {
            gf::Log::error("\nError move_piece: You can't pass by enemies\n");
            return false;
          }
        }
        // On vérifie la présence d'ennemis à droite
        if (source.x != 9)
        {
          if (grid[dest.x+1][i].side != grid[source.x][source.y].side && grid[dest.x+1][i].side != Side::Other)
          {
            gf::Log::error("\nError move_piece: You can't pass by enemies\n");
            return false;
          }
        }
      }
    }
  }
  else // Si c'est toute autre pièce
  {
    // On vérifie qu'on a un déplacement rectiligne de 1 case
    if ( !(abs(source.x - dest.x) == 1 && abs(source.y - dest.y) == 0) && !(abs(source.x - dest.x) == 0 && abs(source.y - dest.y) == 1) )
    {
      gf::Log::error("\n\tError move_piece: Invalid deplacement, must be straight for only 1 compartment\n");
      return false;
    }
  }


  // Cas d'une attaque entre 2 pièces
  if (grid[dest.x][dest.y].side != grid[source.x][source.y].side && grid[dest.x][dest.y].side != Side::Other)
  {
    collision = true;
    // Cas de l'espion qui attaque maréchal
    if ((int) grid[source.x][source.y].rank == 10 && (int) grid[dest.x][dest.y].rank == 1)
    {
      gf::Log::info("\n\tThe assailant won over the Marshal !\n");
      grid[dest.x][dest.y] = grid[source.x][source.y];
      grid[source.x][source.y].side = Side::Other;
      grid[source.x][source.y].rank = Rank::Empty;
      return true;
    }

    // Cas de la bombe et du démineur
    if ((int) grid[dest.x][dest.y].rank == 0 && (int) grid[source.x][source.y].rank == 8)
    {
      gf::Log::info("\n\tOops, assailant bump into bombs !\n");
      grid[dest.x][dest.y] = grid[source.x][source.y];
      grid[source.x][source.y].side = Side::Other;
      grid[source.x][source.y].rank = Rank::Empty;
      return true;
    }

    // Cas de la capture de drapeau
    if ((int) grid[dest.x][dest.y].rank == 11)
    {
      gf::Log::info("\n\tThe assailant found the enemy flag !\n");
      grid[dest.x][dest.y] = grid[source.x][source.y];
      grid[source.x][source.y].side = Side::Other;
      grid[source.x][source.y].rank = Rank::Empty;
      is_end = true;
      return true;
    }

    // Cas standard
    if ((int) grid[dest.x][dest.y].rank < (int) grid[source.x][source.y].rank)
    { // Attaquant moins fort que défenseur
      grid[source.x][source.y].side = Side::Other;
      grid[source.x][source.y].rank = Rank::Empty;
      return true;
    }
    else
    {
      if ((int) grid[dest.x][dest.y].rank > (int) grid[source.x][source.y].rank)
      { // Attaquant plus fort que défenseur
        grid[dest.x][dest.y] = grid[source.x][source.y];
        grid[source.x][source.y].side = Side::Other;
        grid[source.x][source.y].rank = Rank::Empty;
        return true;
      }
      else
      { // Pièces de puissance égales
        grid[source.x][source.y].side = Side::Other;
        grid[source.x][source.y].rank = Rank::Empty;
        grid[dest.x][dest.y].side = Side::Other;
        grid[dest.x][dest.y].rank = Rank::Empty;
        return true;
      }
    }
  }

  // Cas d'un déplacement sur une case vide
  if ((int) grid[dest.x][dest.y].rank == 13)
  {
    collision = false;
    tmp = grid[source.x][source.y];
    grid[source.x][source.y] = grid[dest.x][dest.y];
    grid[dest.x][dest.y] = grid[source.x][source.y];

    return true;
  }

  gf::Log::error("\n\tError move_piece: Unexpected error\n");
  return false;
}


bool s_grid::game_is_end()
{
  return is_end;
}

bool s_grid::had_collision()
{
  return collision;
}
