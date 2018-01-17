#ifndef MESSAGE_H
#define MESSAGE_H

enum class ID_message : int
{
  Accept = 0,
  Initiate = 1,
  Play = 2,
  Move = 3,
  Update = 4,
  End = 5,
  Quit = 6
};

struct Movement
{
  int source;
  int target;
};

struct Resumed_piece
{
  int pos;
  int value;
};

struct Initialize
{
  Resumed_piece pieces[40];
};

struct Update
{
  bool collision;
  Movement movement;
  int enemy_value;
  int result;
};

struct Message
{
  ID_message id;
  union data
  {
    bool result;
    Movement movement;
    Initialize initialize;
  };
};

Message get_message(tcp::socket *socket);
void get_vector_coord(gf::Vector2u *coo2D, int piece_pos, bool inversed);
int get_pos_from_vector(gf::Vector2u *coo2D, bool inversed);

#endif
