#ifndef C_GRID_H
#define C_GRID_H

#include <gf/Entity.h>
#include <gf/TileLayer.h>
#include <gf/RenderTarget.h>
#include <gf/Curves.h>
#include <gf/Color.h>
#include <gf/Shapes.h>
#include <gf/Activities.h>
#include <vector>

#include "c_piece.h"

class Grid : public gf::Entity {
public:
  /// Case sélectionnée (on affiche les déplacements possibles avec cette pièce)
  gf::Vector2i selected = {-1, -1};

  /// Taille (pixels) d'une tile
  static constexpr unsigned TileSize = 64;

  /// Nombre de cases dans la grille
  static constexpr unsigned GridSize = 10;

  Grid(gf::ResourceManager& resources);

  /// Initialisation de la grille
  void createGrid();

  /// Définir la position de la grille
  void setPosition(gf::Vector2f origin);

  /// Position de la grille
  gf::Vector2f getPosition();

  /// Coordonnées de la grille à partir des coordonées de la souris
  gf::Vector2i getPieceCoordsFromMouse(gf::Vector2f coords);

  /// Affichage de la grille
  virtual void render(gf::RenderTarget& target, const gf::RenderStates& states) override;

  /// Mise à jour de la grille
  virtual void update(gf::Time time) override;

  /// Récupère une pièce de la grille
  Piece getPiece(gf::Vector2u coords);

  /// Définit une pièce dans la grille
  bool setPiece(gf::Vector2u coords, Piece p);

  /// Place une pièce à une position aléatoire disponnible
  void setPieceRandom(Piece p);

  /// Supprime une pièce de la grille
  void removePiece(gf::Vector2u coords);

  /// Séléctionne une pièce (retourne false si c'est un échec (case vide par ex))
  bool selectPiece(gf::Vector2u coords);

  /// Déplace et anime la pièce sélectionnée vers coords
  bool moveSelectedPieceTo(gf::Vector2u coords);

  /// Fais la mise à jour reçue par le serveur avec l'animation qui se doit
  bool makeUpdate(gf::Vector2u firstCoords, Piece firstPiece, gf::Vector2u lastCoords, Piece lastPiece);

  /// Indique si une pièce est sélectionnée
  bool isSelected();

private:
  /// Bouge la pièce (en l'animant) de first à last
  bool movePieceTo(gf::Vector2u first, gf::Vector2u last);

  /// Découvre une pièce ennemie (bleue)
  bool discoverPiece(gf::Vector2u coords, Rank r);

  /// Layer : n'est plus directement utilisé : à supprimer
  gf::TileLayer m_layer;

  /// Retourne le tableau des cases accessibles depuis la case en paramètre
  std::vector<gf::Vector2u> getDestinations(gf::Vector2u coords);

  // Indique si une modification a eu lieu sur la grille
  bool modif = false;

  /// Nombre aléatoire entre min et max
  int aleat(int min, int max);

  gf::Vector2i target = {-1, -1};

  /// Tableau représentant la grille
  Piece grid[GridSize][GridSize];

  /// Animation quand on déplace une pièce d'une case à une autre
  gf::MoveToActivity anim;

  bool animEnabled = false;

  gf::Vector2i updateFirstCoords = {-1, -1};
  Piece updateFirstPiece;
  gf::Vector2i updateLastCoords = {-1, -1};
  Piece updateLastPiece;

  gf::Vector2f spritePos;
};

#endif
