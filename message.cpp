#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "packet.h"
#include "message.h"

boost::array<char, 128> get_char_message(tcp::socket &socket, size_t &length) {
  boost::array<char, 128> buf;
  boost::system::error_code error;

  length = socket.read_some(boost::asio::buffer(buf), error);

  if(error) {
    buf[0] = -1;
  }

  return buf;
}

Message get_message(tcp::socket &socket)
{
  size_t length;
  boost::array<char, 128> msg = get_char_message(socket, length);

  struct Message new_message;

  switch(msg[ID_INDEX]) {
    case ID_message::Accept:
      new_message.id = ID_message::Accept;
      new_message.data.accept = msg[1] == 1;
      break;
    case ID_message::Initiate:
      new_message.id = ID_message::Initiate;
      int piece_index = 0;
      for(int i = 1; i < PLAYER_MAX_PIECES*2 + 1; i += 2) {
        new_message.data.initiate.pieces[piece_index].pos = (int)(msg[i]);
        new_message.data.initiate.pieces[piece_index].value = (int)(msg[i + 1]);
        piece_index++;
      }
      break;
    case ID_message::Play:
      new_message.id = ID_message::Play;
      break;
    case ID_message::Move:
      new_message.id = ID_message::Move;
      new_message.move.source = (int)(msg[1]);
      new_message.move.target = (int)(msg[2]);
      break;
    case ID_message::Update:
    {
      new_message.id = ID_message::Update;
      new_message.update.collision = msg[UPDATE_INDEX] == COLLISION;

      new_message.update.movement.source = msg[2];
      new_message.update.movement.target = msg[3];

      if(new_message.update.collision) {
        new_message.udpate.enemy_value = msg[4];
        new_message.update.result = (Result)(msg[5]);
      }
    }
    break;
    case ID_message::End:
      new_message.id = ID_message::End;
      new_message.end = (Result)(msg[1]);
      break;
    case ID_message::Quit:
      new_message.id = ID_message::Quit;
      break;
  }

  return new_message;
}

void get_vector_coord(gf::Vector2u *coo2D, int piece_pos, bool inversed)
{
  if (!inversed)
  {
    coo2D->x = 9 - (piece_pos % 10);
    coo2D->y = 9 - piece_pos / 10;
  }
  else
  {
    coo2D->x = piece_pos % 10;
    coo2D->y = piece_pos / 10;
  }
}

int get_pos_from_vector(gf::Vector2u *coo2D, bool inversed)
{
  if (!inversed)
  {
    return (9 - coo2D->x)+(9- coo2D->y)*10;
  }
  else
  {
    return coo2D->x + coo2D->y*10;
  }
}
