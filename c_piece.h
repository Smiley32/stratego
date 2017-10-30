#ifndef C_PIECE_H
#define C_PIECE_H

#include <gf/RenderWindow.h>
#include <gf/Sprite.h>
#include <gf/ResourceManager.h>
#include <gf/Texture.h>

/**
 * Camp de la piece : Other pour les éléments neutres (lac / case vide)
 */
enum class Side {Red, Blue, Other};

/**
 * Rang de la piece :
 * Bomb (6) - Ne bouge pas
 * Marshal (1) - (no 10)
 * General (1) - (no 9)
 * Colonel (2) - (no 8)
 * Major (3) - (no 7)
 * Captain (4) - (no 6)
 * Lieutenant (4) - (no 5)
 * Sergeant (4) - (no 4)
 * Miner (5) - (no 3)
 * Scout (8) - (no 2)
 * Spy (1) - (no 1)
 * Flag (1) - Ne bouge pas
 *
 * Water - Case d'eau (8 en tout) - non accessibles
 * Empty - Cases vides
 */
enum class Rank : int {
  Bomb = 0,
  Marshal = 1,
  General = 2,
  Colonel = 3,
  Major = 4,
  Captain = 5,
  Lieutenant = 6,
  Sergeant = 7,
  Miner = 8,
  Scout = 9,
  Spy = 10,
  Flag = 11,
  Water = 12,
  Empty = 13
};

struct Piece {
  Side side;
  Rank rank;
};

#endif
