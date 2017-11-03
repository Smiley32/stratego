#ifndef SPIECE_H
#define SPIECE_H


enum class Side
{
  Red,
  Blue,
  Other
};

enum class Rank : int
{
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

struct Piece
{
  Side side;
  Rank rank;
};

class s_piece
{
  public:


  private:


};
#endif
