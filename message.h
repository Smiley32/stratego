#ifndef MESSAGE_H
#define MESSAGE_H

#define ID_INDEX 0

#define PLAYER_MAX_PIECES 40

#define UPDATE_INDEX 1
#define COLLISION 1

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

enum class Result : int
{
  Lose = 0,
  Win = 1,
  Lose = 0
};

typedef struct Movement Movement;
struct Movement
{
  int source;
  int target;
};

typedef struct Resumed_piece Resumed_piece;
struct Resumed_piece
{
  int pos;
  int value;
};

typedef struct Initialize Initialize;
struct Initialize
{
  Resumed_piece pieces[PLAYER_MAX_PIECES];
};

typedef struct Update Update;
struct Update
{
  bool collision;
  Movement movement;
  int enemy_value;
  Result result;
};

typedef struct Message Message;
struct Message
{
  ID_message id;
  union data
  {
    bool accept;
    Initialize initiate;
    // play: rien
    Movement move;
    Update update;
    Result end;
    // quit: rien
  };
};

/**
 * Attends un message (ne gère pas la réception de messages concaténés)
 * 
 * @param tcp::socket *socket Socket sur laquelle lire le message
 * @return Message    Message retourné
 */
Message get_message(tcp::socket &socket);
void get_vector_coord(gf::Vector2u *coo2D, int piece_pos, bool inversed);
int get_pos_from_vector(gf::Vector2u *coo2D, bool inversed);

#endif
