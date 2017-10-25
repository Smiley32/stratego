#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "s_grid.h"

using boost::asio::ip::tcp;

bool s_grid::checkMovement(s_piece target, int x, int y)
{
  // Vérification coordonnées cibles dans l'espace de jeu
  if (x < 0 || y < 0 || x > 9 || y > 9)
  {
    return false;
  }

  // Si on essaye de déplacer une bombe ou un drapeau
  if (abs(value) == 11 || abs(value) == 12)
  {
    return false;
  }

  // Vérification mouvement spécial éclaireur
  if (abs(value) == 8)
  {
      if ((abs(target.getPosX() - x) > 1 && abs(target.getPosY() - y) == 0) || (abs(target.getPosY() - y) > 1 && (abs(target.getPosX() - x) == 0))
      {
        return true;
      }

      return false;
  }
  else
  {
    // Vérification le déplacement n'est pas plus de 1
    if ((abs(target.getPosX() - x) == 1 && abs(target.getPosY() - y) == 0) || (abs(target.getPosY() - y) == 1 && (abs(target.getPosX() - x) == 0))
    {
      return false;
    }
  }

  //TODO Vérification case libre

  return true;
}
