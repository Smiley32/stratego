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

#define DEFAULT_PIECE_WIDTH 64

#define DEFAULT_SELECT_X 96
#define DEFAULT_SELECT_Y 804

#define DEFAULT_FIRST_BAR_X 36
#define DEFAULT_FIRST_BAR_Y 32

#define DEFAULT_SECOND_BAR_X 864
#define DEFAULT_SECOND_BAR_Y 36

gf::Vector2u get_current_position(gf::Vector2u default_pos, double scale);

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
 * Scout (8) - (no 2) - Bouge en ligne
 * Spy (1) - (no 1)
 * Flag (1) - Ne bouge pas
 *
 * Water - Case d'eau (8 en tout) - non accessibles
 * Empty - Cases vides
 * Unknown - Pièce inconnue
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
  Empty = 13,
  Unknown = 14
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
  unsigned TileSize = DEFAULT_PIECE_WIDTH;
  static constexpr unsigned NbPieces = 12;

  // Indice de la pièce selectionnée (-1 si aucune)
  int selected = -1;

  Selection(gf::ResourceManager& resources);

  void setPosition(gf::Vector2f origin);
  gf::Vector2f getPosition();

  gf::Vector2i getPieceCoordsFromMouse(gf::Vector2f coords);

  virtual void render(gf::RenderTarget& target, const gf::RenderStates& states) override;

  void selectPiece(unsigned int pieceNumber);
  void takeOnePiece(unsigned int pieceNumber);

  Piece getPiece(gf::Vector2u coords);

  /// Récupère un indice aléatoire d'une pièce qu'il reste à placer (Empty si aucune)
  Piece getRandomPiece();

  void addPiece(Piece p);

  void updateMouseCoords(gf::Vector2i coords);

  /// Indique si il reste des pièces à placer
  bool isEmpty();

  /// Changer la proportion
  void update_scale(double new_scale);

private:
  /// Nombre aléatoire entre min et max
  int aleat(int min, int max);

  gf::TileLayer m_layer;
  // Nombre de chaque pièce restant à placer
  int nbPieces[NbPieces] = {6, 1, 1, 2, 3, 4, 4, 4, 5, 8, 1, 1};
  Piece grid[NbPieces];

  gf::Vector2i mouseCoords;

  double scale;
  gf::Vector2f position;

  enum {
    Select,
    Display
  } state;
};

#endif
