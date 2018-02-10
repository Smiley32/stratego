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

void Selection::makeVertical() {
  vertical = true;
}

void Selection::makeBlueSide() {
  blueSide = true;
}

void Selection::setPosition(gf::Vector2f origin) {
  position = origin;
}

gf::Vector2f Selection::getPosition() {
  return position;
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
      // std::cout << "nb aleat : " << x << std::endl;
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

void Selection::update_scale(double new_scale) {
  scale = new_scale;
  TileSize = DEFAULT_PIECE_WIDTH * scale;
}

void Selection::selectPiece(unsigned int pieceNumber) {
  if(pieceNumber < NbPieces && nbPieces[pieceNumber] > 0) {
    selected = pieceNumber;
    
    // std::cout << "Selected : " << nbPieces[pieceNumber] << std::endl;
  } else {
    selected = -1;
  }
}

void Selection::takeOnePiece(unsigned int pieceNumber) {
  nbPieces[pieceNumber]--;
  // std::cout << "nbPieces restantes : " << nbPieces[pieceNumber] << std::endl;
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

void Selection::render(gf::RenderTarget& target, const gf::RenderStates& states) {

  // Affichage du fond
  gf::Texture background_texture;
  background_texture.loadFromFile("select.png");

  gf::Sprite background;
  background.setTexture(background_texture);

  if(vertical) {
    background.setRotation(3.14159265358979323846 / 2);
    background.setPosition({getPosition().x + TileSize, getPosition().y});
  } else {
    background.setPosition({getPosition().x, getPosition().y});
  }

  // background.setScale(scale);
  target.draw(background);

  gf::Texture texture;
  if(blueSide) {
    texture.loadFromFile("pieces_blue.png");
  } else {
    texture.loadFromFile("pieces.png");
  }
  
  gf::Font font;
  font.loadFromFile("16_DejaVuSans.ttf");

  for(unsigned x = 0; x < NbPieces; x++) {
    gf::Vector2u coords(x, 0);
    int r = (int)grid[x].rank;

    // On affiche des cases vides si la pièce n'est plus disponible
    if(nbPieces[x] == 0) {
      r = (int)Rank::Empty;
    }

    gf::Sprite sprite(texture, gf::RectF( ((r * DEFAULT_PIECE_WIDTH) % 256) / 256.0, (((r * DEFAULT_PIECE_WIDTH) / 256) * DEFAULT_PIECE_WIDTH) / 256.0, DEFAULT_PIECE_WIDTH / 256.0, DEFAULT_PIECE_WIDTH / 256.0));
    if(vertical) {
      sprite.setPosition({getPosition().x, getPosition().y + (x * TileSize)});
    } else {
      sprite.setPosition({getPosition().x + (x * TileSize), getPosition().y});
    }
    
    // sprite.setScale(scale);
    target.draw(sprite, states);

    // Affichage du nombre de pièces restantes
    gf::Text txt(std::to_string(nbPieces[x]), font);
    txt.setCharacterSize(20);
    if(vertical) {
      txt.setPosition({getPosition().x + TileSize, getPosition().y + (x * TileSize) + 20});
    } else {
      txt.setPosition({getPosition().x + (x * TileSize) + 20, getPosition().y + TileSize + 20});
    }
    target.draw(txt, states);

    // m_layer.setTile(coords, static_cast<int>(grid[x].rank));
  }

  // Affichage de la pièce selectionnée
  if(selected != -1) {
    // std::cout << "(x,y): (" << mouseCoords.x << "," << mouseCoords.y << ")" << std::endl;
    gf::Sprite sprite(texture, gf::RectF( ((selected * DEFAULT_PIECE_WIDTH) % 256) / 256.0, (((selected * DEFAULT_PIECE_WIDTH) / 256) * DEFAULT_PIECE_WIDTH) / 256.0, DEFAULT_PIECE_WIDTH / 256.0, DEFAULT_PIECE_WIDTH / 256.0));
    sprite.setPosition({(float)(mouseCoords.x - TileSize / 2), (float)(mouseCoords.y - TileSize / 2)});
    // sprite.setScale(scale);

    target.draw(sprite, states);
  }

  // target.draw(m_layer);
}
