#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "s_piece.h"

using boost::asio::ip::tcp;


int s_piece::getValue()
{
  return value;
}

int s_piece::getPosX()
{
  return posX;
}

int s_piece::getPosY()
{
  return posY;
}

void s_piece::setValue(int v)
{
  value = v;
}

void s_piece::setPosX(int x)
{
  posX = x;
}

void s_piece::setPosY(int y)
{
  posY = y;
}

bool s_piece::checkMovement(int x, int y)
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
      if ((abs(posX - x) > 1 && abs(posY - y) == 0) || (abs(posY - y) > 1 && (abs(posX - x) == 0))
      {
        return true;
      }

      return false;
  }
  else
  {
    // Vérification le déplacement n'est pas plus de 1
    if ((abs(posX - x) == 1 && abs(posY - y) == 0) || (abs(posY - y) == 1 && (abs(posX - x) == 0))
    {
      return false;
    }
  }

  return true;
}
