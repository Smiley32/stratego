#include "c_piece.h"

#include <iostream>

Selection::Selection(gf::ResourceManager& resources)
: m_layer({NbPieces, 1})
{
  m_layer.setTileSize({TileSize, TileSize});
  m_layer.setTexture(resources.getTexture("pieces.png"));

  for(unsigned x = 0; x < NbPieces; x++) {
    grid[x].side = Side::Red;
    grid[x].rank = (Rank)x;
  }

  m_layer.clear();
  for(unsigned x = 0; x < NbPieces; x++) {
    gf::Vector2u coords(x, 0);
    m_layer.setTile(coords, static_cast<int>(grid[x].rank));
  }
}

void Selection::setPosition(gf::Vector2f origin) {
  m_layer.setPosition(origin);
}

gf::Vector2f Selection::getPosition() {
  return m_layer.getPosition();
}

Piece Selection::getPiece(gf::Vector2u coords) {
  int tile = m_layer.getTile(coords);
  Piece p;
  if(tile >= 12) {
    p.side = Side::Other;
  } else {
    p.side = Side::Red;
  }
  p.rank = (Rank)tile;

  return p;
}

Piece Selection::getRandomPiece() {
  Piece p;

  if(isEmpty()) {
    p.side = Side::Other;
    p.rank = Rank::Empty;
  } else {
    int x;
    do {
      x = aleat(0, NbPieces - 1);
      std::cout << "nb aleat : " << x << std::endl;
    } while(nbPieces[x] == 0);
    
    p.side = Side::Red;
    p.rank = (Rank)x;
  }

  return p;
}

gf::Vector2i Selection::getPieceCoordsFromMouse(gf::Vector2f coords) {
  if(coords.x < getPosition().x || coords.y < getPosition().y || coords.x > getPosition().x + (NbPieces * TileSize) || coords.y > getPosition().y + TileSize) {
    return {-1, -1};
  }

  return {(int)((coords.x - getPosition().x) / TileSize), (int)((coords.y - getPosition().y) / TileSize)};
}

void Selection::selectPiece(unsigned int pieceNumber) {
  if(pieceNumber < NbPieces && nbPieces[pieceNumber] > 0) {
    selected = pieceNumber;
    
    std::cout << "Selected : " << nbPieces[pieceNumber] << std::endl;
  } else {
    selected = -1;
  }
}

void Selection::takeOnePiece(unsigned int pieceNumber) {
  nbPieces[pieceNumber]--;
  std::cout << "nbPieces restantes : " << nbPieces[pieceNumber] << std::endl;
}

void Selection::addPiece(Piece p) {
  nbPieces[(int)p.rank]++;
}

void Selection::updateMouseCoords(gf::Vector2i coords) {
  mouseCoords = coords;
}

bool Selection::isEmpty() {
  for(int i = 0; i < NbPieces; i++) {
    if(nbPieces[i] != 0) {
      return false;
    }
  }
  return true;
}

int Selection::aleat(int min, int max) {
  return rand() % (max - min + 1) + min;
}

