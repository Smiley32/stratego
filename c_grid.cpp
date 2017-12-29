#include "c_grid.h"

#include <iostream>

#include <gf/Easings.h>

Grid::Grid(gf::ResourceManager& resources)
: m_layer({GridSize, GridSize})
, anim({0, 0}, {0, 0}, spritePos, gf::seconds(0.5), gf::Ease::elasticInOut)
{
  m_layer.setTileSize({TileSize, TileSize});
  m_layer.setTexture(resources.getTexture("pieces.png"));

  updateFirstPiece.side = Side::Other;
  updateFirstPiece.rank = Rank::Unknown;

  updateLastPiece.side = Side::Other;
  updateLastPiece.rank = Rank::Unknown;

  // Initialisation de la grille
  for(unsigned x = 0; x < GridSize; x++) {
    for(unsigned y = 0; y < GridSize; y++) {
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

  // Ajout des pièces de l'adversaire
  for(unsigned x = 0; x < GridSize; x++) {
    for(unsigned y = 0; y < 4; y++) {
      grid[x][y].side = Side::Blue;
      grid[x][y].rank = Rank::Unknown;
    }
  }
}

void Grid::createGrid() {
  m_layer.clear();

  for(unsigned x = 0; x < GridSize; x++) {
    for(unsigned y = 0; y < GridSize; y++) {
      gf::Vector2u coords(x, y);
      m_layer.setTile(coords, static_cast<int>(grid[x][y].rank));
    }
  }
}

void Grid::setPosition(gf::Vector2f origin) {
  m_layer.setPosition(origin);
}

gf::Vector2f Grid::getPosition() {
  return m_layer.getPosition();
}

gf::Vector2i Grid::getPieceCoordsFromMouse(gf::Vector2f coords) {
  if(coords.x < getPosition().x || coords.y < getPosition().y || coords.x > getPosition().x + (GridSize * TileSize) || coords.y > getPosition().y + (GridSize * TileSize)) {
    return {-1, -1};
  }

  return {(int)((coords.x - getPosition().x) / TileSize), (int)((coords.y - getPosition().y) / TileSize)};
}

Piece Grid::getPiece(gf::Vector2u coords) {
  return grid[coords.x][coords.y];
}

void Grid::removePiece(gf::Vector2u coords) {
  grid[coords.x][coords.y].side = Side::Other;
  grid[coords.x][coords.y].rank = Rank::Empty;
}

bool Grid::setPiece(gf::Vector2u coords, Piece p) {
  if(grid[coords.x][coords.y].rank == Rank::Empty && coords.y >= 6) {
    grid[coords.x][coords.y].rank = p.rank;
    grid[coords.x][coords.y].side = p.side;
    modif = true;
    std::cout << "..." << coords.x << "," << coords.y << " ; " << (int)grid[coords.x][coords.y].rank << std::endl;
    return true;
  }
  return false;
}

int Grid::aleat(int min, int max) {
  return rand() % (max - min + 1) + min;
}

void Grid::setPieceRandom(Piece p) {
  int pos = aleat(60, 99);

  while(!setPiece({pos % GridSize, pos / GridSize}, p)) {
    pos = aleat(60, 99);
  }
}

void Grid::render(gf::RenderTarget& target, const gf::RenderStates& states) {
  // std::cout << "Dessin..." << std::endl;
  // target.draw(m_layer, states);

  // Tracé de la grille
  for(unsigned x = 0; x <= GridSize; x++) {
    // Tracé des colonnes
    gf::Line colonne({getPosition().x + (x * TileSize), getPosition().y}, {getPosition().x + (x * TileSize), getPosition().y + (GridSize * TileSize)});
    colonne.setColor(gf::Color::Black);
    target.draw(colonne, states);

    // Tracé des lignes
    gf::Line line({getPosition().x, getPosition().y + (x * TileSize)}, {getPosition().x + (GridSize * TileSize), getPosition().y + (x * TileSize)});
    line.setColor(gf::Color::Black);
    target.draw(line, states);
  }

  if(selected.x != -1 && selected.y != -1) {
    // Calcul des destinations
    std::vector<gf::Vector2u> destinations = getDestinations({(unsigned)selected.x, (unsigned)selected.y});

    for(int i = 0; i < destinations.size(); i++) {
      // Il faut marquer la case
      // std::cout << "Destination (" << destinations[i].x << " ; " << destinations[i].y << ")" << std::endl;
      gf::RectangleShape marque({TileSize, TileSize});
      marque.setPosition({getPosition().x + (destinations[i].x * TileSize), getPosition().y + (destinations[i].y * TileSize)});
      marque.setColor({(0.0F), (0.0F), (1.0F), (0.5F)});
      target.draw(marque, states);
    }
  }

  gf::Texture texture;
  texture.loadFromFile("pieces.png");

  gf::Texture blueTexture;
  blueTexture.loadFromFile("pieces_blue.png");

  for(unsigned x = 0; x < GridSize; x++) {
    for(unsigned y = 0; y < GridSize; y++) {
      // On n'affiche pas la pièce sélectionnée si on a commencé son animation
      if(animEnabled &&
          ((updateFirstCoords.x != -1 && selected.x == x && selected.y == y) ||
           (updateFirstCoords.x == x && updateFirstCoords.y == y))) {
        continue;
      }

      gf::Vector2u coords(x, 0);
      int r = (int)grid[x][y].rank;

      gf::Texture *t;
      if(grid[x][y].side == Side::Blue) {
        t = &blueTexture;
      } else {
        t = &texture;
      }

      gf::Sprite sprite(*t, gf::RectF( ((r * TileSize) % 256) / 256.0, (((r * TileSize) / 256) * TileSize) / 256.0, TileSize / 256.0, TileSize / 256.0));
      sprite.setPosition({getPosition().x + (x * TileSize), getPosition().y + (y * TileSize)});
      target.draw(sprite, states);
    }
  }

  // Animation de la pièce
  if(animEnabled) {
    int r;
    if(updateFirstCoords.x != -1) {
      r = (int)grid[updateFirstCoords.x][updateFirstCoords.y].rank;
    } else {
      r = (int)grid[selected.x][selected.y].rank;
    }
    
    gf::Sprite sprite(texture, gf::RectF( ((r * TileSize) % 256) / 256.0, (((r * TileSize) / 256) * TileSize) / 256.0, TileSize / 256.0, TileSize / 256.0));
    sprite.setPosition(spritePos);
    target.draw(sprite, states);
    std::cout << "<" << r << "> affichage..." << spritePos.x << " ; " << spritePos.y << std::endl;
  }
}

void Grid::update(gf::Time time) {
  if(modif) {
    std::cout << "update" << std::endl;
    modif = false;

    m_layer.clear();
    for(unsigned x = 0; x < GridSize; x++) {
      for(unsigned y = 0; y < GridSize; y++) {
        gf::Vector2u coords(x, y);
        m_layer.setTile(coords, static_cast<int>(grid[x][y].rank));
        // std::cout << "[" << coords.x << ", " << coords.y << "] - " << m_layer.getTile(coords) << std::endl;
      }
    }
  }

  if(animEnabled) {
    if(anim.run(time) == gf::ActivityStatus::Finished) {
      animEnabled = false;

      if(updateFirstCoords.x != -1) {
        // L'animation était celle d'un update

        // Mise à jour de la grille
        grid[updateLastCoords.x][updateLastCoords.y].rank = updateLastPiece.rank;
        grid[updateLastCoords.x][updateLastCoords.y].side = updateLastPiece.side;
        grid[updateFirstCoords.x][updateFirstCoords.y].rank = Rank::Empty;
        grid[updateFirstCoords.x][updateFirstCoords.y].side = Side::Other;

        // Réinitialisation des valeurs pour la prochaine animation
        updateFirstCoords = {-1, -1};
        updateLastCoords = {-1, -1};
        updateFirstPiece.rank = Rank::Unknown;
        updateLastPiece.rank = Rank::Unknown;
        updateFirstPiece.side = Side::Other;
        updateLastPiece.side = Side::Other;
      } else {
        grid[target.x][target.y] = grid[selected.x][selected.y];
        grid[selected.x][selected.y].rank = Rank::Empty;
        grid[selected.x][selected.y].side = Side::Other;
        selected = {-1, -1};
      }
      target = {-1, -1};
    }
  }
}

bool Grid::selectPiece(gf::Vector2u coords) {
  if(coords.x > GridSize || coords.y > GridSize) {
    selected = {-1, -1};
    return false;
  }

  if(grid[coords.x][coords.y].side == Side::Blue || grid[coords.x][coords.y].rank == Rank::Water || grid[coords.x][coords.y].rank == Rank::Empty) {
    selected = {-1, -1};
    return false;
  }

  selected = coords;
  return true;
}

bool Grid::moveSelectedPieceTo(gf::Vector2u coords) {
  if(!isSelected()) {
    return false;
  }

  // std::cout << "Salut :)" << std::endl;
  std::vector<gf::Vector2u> destinations = getDestinations(selected);

  for(int i = 0; i < destinations.size(); i++) {
    std::cout << destinations[i].x << " - " << destinations[i].y << std::endl;
    if(coords.x == destinations[i].x && coords.y == destinations[i].y) {
      // On commence l'animation
      animEnabled = true;
      target = coords;

      anim.restart();
      anim.setOrigin({getPosition().x + (selected.x * TileSize), getPosition().y + (selected.y * TileSize)});
      anim.setTarget({getPosition().x + (coords.x * TileSize), getPosition().y + (coords.y * TileSize)});
      anim.setDuration(gf::seconds(1));
      return true;
    }
  }

  return false;
}

bool Grid::makeUpdate(gf::Vector2u firstCoords, Piece firstPiece, gf::Vector2u lastCoords, Piece lastPiece) {
  // TODO: vérifications de firstCoords et lastCoords
  updateFirstCoords = firstCoords;
  updateFirstPiece = firstPiece;
  updateLastCoords = lastCoords;
  updateLastPiece = lastPiece;

  if(discoverPiece(firstCoords, firstPiece.rank)) {
    return movePieceTo(firstCoords, lastCoords);
  } else {
    std::cout << "Erreur : makeUpdate" << std::endl;
    return false;
  }
}

bool Grid::movePieceTo(gf::Vector2u first, gf::Vector2u last) {
  // TODO: vérifications de first et last

  updateFirstCoords = first;
  updateLastCoords = last;

  animEnabled = true;
  target = last;

  anim.restart();
  anim.setOrigin({getPosition().x + (first.x * TileSize), getPosition().y + (first.y * TileSize)});
  anim.setTarget({getPosition().x + (last.x * TileSize), getPosition().y + (last.y * TileSize)});
  anim.setDuration(gf::seconds(1));

  return true;
}

bool Grid::discoverPiece(gf::Vector2u coords, Rank r) {
  // TODO: vérification de coords

  grid[coords.x][coords.y].rank = r;
}

bool Grid::isSelected() {
  return selected.x != -1 && selected.y != -1;
}

std::vector<gf::Vector2u> Grid::getDestinations(gf::Vector2u coords) {

  std::vector<gf::Vector2u> destinations;

  switch(grid[coords.x][coords.y].rank) {
    case Rank::Water:
    case Rank::Empty:
    case Rank::Bomb:
    case Rank::Flag:
      return destinations;
      break;
    case Rank::Scout:
    {
      // Le scout peut bouger d'autant qu'il veut en ligne, sans passer au dessus des autres pièces
      // Parcours des cases autour
      int x = coords.x - 1;
      while(x >= 0) {
        if(grid[x][coords.y].rank == Rank::Empty || grid[x][coords.y].side == Side::Blue) {
          destinations.push_back({x, coords.y});
        }
        
        if(grid[x][coords.y].rank != Rank::Empty || grid[x][coords.y].side == Side::Blue) {
          // On arrête de parcourir les cases dans cette direction
          break;
        }
        x--;
      }

      x = coords.x + 1;
      while(x < GridSize) {
        if(grid[x][coords.y].rank == Rank::Empty || grid[x][coords.y].side == Side::Blue) {
          destinations.push_back({x, coords.y});
        }
        
        if(grid[x][coords.y].rank != Rank::Empty || grid[x][coords.y].side == Side::Blue) {
          // On arrête de parcourir les cases dans cette direction
          break;
        }
        x++;
      }

      int y = coords.y - 1;
      while(y >= 0) {
        if(grid[coords.x][y].rank == Rank::Empty || grid[coords.x][y].side == Side::Blue) {
          destinations.push_back({coords.x, y});
        }
        
        if(grid[coords.x][y].rank != Rank::Empty || grid[coords.x][y].side == Side::Blue) {
          // On arrête de parcourir les cases dans cette direction
          break;
        }
        y--;
      }

      y = coords.y + 1;
      while(y < GridSize) {
        if(grid[coords.x][y].rank == Rank::Empty || grid[coords.x][y].side == Side::Blue) {
          destinations.push_back({coords.x, y});
        }
        
        if(grid[coords.x][y].rank != Rank::Empty || grid[coords.x][y].side == Side::Blue) {
          // On arrête de parcourir les cases dans cette direction
          break;
        }
        y++;
      }
    }
      break;
    default:
      // Parcours uniquement des 4 cases autour
      if(coords.x > 0) {
        if(grid[coords.x - 1][coords.y].rank == Rank::Empty || grid[coords.x - 1][coords.y].side == Side::Blue) {
          destinations.push_back({coords.x - 1, coords.y});
        }
      }

      if(coords.x + 1 < GridSize) {
        if(grid[coords.x + 1][coords.y].rank == Rank::Empty || grid[coords.x + 1][coords.y].side == Side::Blue) {
          destinations.push_back({coords.x + 1, coords.y});
        }
      }

      if(coords.y > 0) {
        if(grid[coords.x][coords.y - 1].rank == Rank::Empty || grid[coords.x][coords.y - 1].side == Side::Blue) {
          destinations.push_back({coords.x, coords.y - 1});
        }
      }

      if(coords.x + 1 < GridSize) {
        if(grid[coords.x][coords.y + 1].rank == Rank::Empty || grid[coords.x][coords.y + 1].side == Side::Blue) {
          destinations.push_back({coords.x, coords.y + 1});
        }
      }

      break;
  }

  return destinations;
}
