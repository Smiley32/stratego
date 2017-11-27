#ifndef C_PIECE_H
#define C_PIECE_H

#include <gf/Entity.h>
#include <gf/TileLayer.h>
#include <gf/ResourceManager.h>
#include <gf/Texture.h>
#include <gf/RenderTarget.h>
#include <gf/Sprite.h>
#include <gf/VectorOps.h>
#include <gf/Transform.h>
#include <gf/Text.h>

#include <string>

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

/**
 * Outil de sélection des pièces (avant le jeu)
 */
class Selection : public gf::Entity {
public:
  static constexpr unsigned TileSize = 64;
  static constexpr unsigned NbPieces = 12;

  // Indice de la pièce selectionnée (-1 si aucune)
  int selected = -1;

  Selection(gf::ResourceManager& resources);

  void setPosition(gf::Vector2f origin);
  gf::Vector2f getPosition();

  gf::Vector2i getPieceCoordsFromMouse(gf::Vector2f coords);

  virtual void render(gf::RenderTarget& target, const gf::RenderStates& states) override;

  Piece getPiece(gf::Vector2u coords);

  void selectPiece(unsigned int pieceNumber);
  void takeOnePiece(unsigned int pieceNumber);

  void addPiece(Piece p);

  void updateMouseCoords(gf::Vector2i coords);

  /// Indique si il reste des pièces à placer
  bool isEmpty();

private:
  gf::TileLayer m_layer;
  // Nombre de chaque pièce restant à placer
  int nbPieces[NbPieces] = {6, 1, 1, 2, 3, 4, 4, 4, 5, 8, 1, 1};
  Piece grid[NbPieces];

  gf::Vector2i mouseCoords;
};

#endif
