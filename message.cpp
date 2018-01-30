#include "message.h"

void send_packet(tcp::socket* socket, Packet &p)
{
  char *data = (char*)p.getData();

  // TODO ajout de logs

  boost::system::error_code ignored_error;
  boost::asio::write(*socket, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
}

boost::array<char, 128> get_char_message(tcp::socket &socket, size_t &length)
{
  boost::array<char, 128> buf;
  boost::system::error_code error;

  length = socket.read_some(boost::asio::buffer(buf), error);

  if(error)
  {
    buf[0] = -1;
  }

  return buf;
}

Message get_message(tcp::socket &socket)
{
  size_t length;
  boost::array<char, 128> msg = get_char_message(socket, length);

  struct Message new_message;

  switch((int) msg[ID_INDEX])
  {
    case (int) ID_message::Accept:
    {
      new_message.id = ID_message::Accept;
      new_message.data.accept = msg[1] == 1;
    }
    break;
    case (int) ID_message::Initiate:
    {
      new_message.id = ID_message::Initiate;
      int piece_index = 0;
      for(int i = 1; i < PLAYER_MAX_PIECES*2 + 1; i += 2) {
        new_message.data.initiate.pieces[piece_index].pos = (int)(msg[i]);
        new_message.data.initiate.pieces[piece_index].value = (int)(msg[i + 1]);
        piece_index++;
      }
    }
    break;
    case (int) ID_message::Play:
    {
      new_message.id = ID_message::Play;
    }
    break;
    case (int) ID_message::Move:
    {
      new_message.id = ID_message::Move;
      new_message.data.move.source = (int)(msg[1]);
      new_message.data.move.target = (int)(msg[2]);
    }
    break;
    case (int) ID_message::Update:
    {
      new_message.id = ID_message::Update;
      new_message.data.update.collision = msg[UPDATE_INDEX] == COLLISION;

      new_message.data.update.movement.source = msg[2];
      new_message.data.update.movement.target = msg[3];

      if(new_message.data.update.collision) {
        new_message.data.update.enemy_value = msg[4];
        new_message.data.update.result = (Result)(msg[5]);
      }
    }
    break;
    case (int) ID_message::End:
    {
      new_message.id = ID_message::End;
      new_message.data.end = (Result)(msg[1]);
    }
    break;
    case (int) ID_message::Quit:
    {
      new_message.id = ID_message::Quit;
    }
    break;
  }

  return new_message;
}

void send_message(tcp::socket &socket, Message our_message)
{
  Packet p;

  p.append((int) our_message.id);

  switch (our_message.id)
  {
    case ID_message::Accept:
    {
      gf::Log::info("\nSend signal Accept (0) with value %s\n", our_message.data.accept?"true":"false");
      p.append(our_message.data.accept);
    }
    break;
    case ID_message::Initiate:
    {
      gf::Log::info("\nSend signal Initiate (1) with values:\n");

      for (size_t i = 0; i <= PLAYER_MAX_PIECES; i++)
      {
        gf::Log::info("\t%zu:Piece %d at position %d\n", i, our_message.data.initiate.pieces[i].value, our_message.data.initiate.pieces[i].pos);
        p.append(our_message.data.initiate.pieces[i].value);
        p.append(our_message.data.initiate.pieces[i].pos);
      }
    }
    break;
    case ID_message::Play:
    {
      gf::Log::info("\nSend signal Play (2)");
    }
    break;
    case ID_message::Move:
    {
      gf::Log::info("\nSend signal Move (3) with source %d and target %d\n", our_message.data.move.source, our_message.data.move.target);
      p.append(our_message.data.move.source);
      p.append(our_message.data.move.target);
    }
    break;
    case ID_message::Update:
    {
      p.append(our_message.data.update.collision);
      p.append(our_message.data.update.movement.source);
      p.append(our_message.data.update.movement.target);

      if (our_message.data.update.collision)
      {
        gf::Log::info("\nSend signal Update (4) with collision, movement is %d from %d, enemy piece is %d, the result for you is %d", our_message.data.update.movement.source, our_message.data.update.movement.target, our_message.data.update.enemy_value, (int) our_message.data.update.result);
        p.append(our_message.data.update.enemy_value);
        p.append((int) our_message.data.update.result);
      }
      else
      {
        gf::Log::info("\nSend signal Update (4) without collision, movement is %d from %d\n", our_message.data.update.movement.source, our_message.data.update.movement.target);
      }
    }
    break;
    case ID_message::End:
    {
      gf::Log::info("\nSend signal End (5), the result for you is %d\n", (int) our_message.data.end);
      p.append((int) our_message.data.end);
    }
    break;
    case ID_message::Quit:
    {
      gf::Log::info("\nSend signal Quit (6)\n");
    }
    break;
  }

  send_packet(&socket, p);
}

Message create_accept_message(bool accepted)
{
  Message new_message;
  new_message.id = ID_message::Accept;
  new_message.data.accept = accepted;

  return new_message;
}

Message create_initiate_message(struct Initialize pieces)
{
  Message new_message;
  new_message.id = ID_message::Initiate;
  new_message.data.initiate = pieces;

  return new_message;
}

Message create_play_message()
{
  Message new_message;
  new_message.id = ID_message::Play;

  return new_message;
}

Message create_move_message(struct Movement movement)
{
  Message new_message;
  new_message.id = ID_message::Update;
  new_message.data.move = movement;

  return new_message;
}

Message create_update_message(struct Movement movement)
{
  Message new_message;
  new_message.id = ID_message::Update;
  new_message.data.update.collision = false;
  new_message.data.update.movement = movement;

  return new_message;
}

Message create_update_message(struct Movement movement, int enemy_value, Result result)
{
  Message new_message = create_update_message(movement);
  new_message.data.update.collision = true;
  new_message.data.update.enemy_value = enemy_value;
  new_message.data.update.result = result;

  return new_message;
}

Message create_end_message(bool result)
{
  Message new_message;
  new_message.id = ID_message::End;
  new_message.data.accept = result;

  return new_message;
}

Message create_quit_message()
{
  Message new_message;
  new_message.id = ID_message::Quit;

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

Resumed_piece create_resumed_piece(gf::Vector2u *coo2D, int value, bool inversed)
{
  Resumed_piece new_piece;

  new_piece.pos = get_pos_from_vector(coo2D, inversed);
  new_piece.value = value;

  return new_piece;
}

Movement create_movement(gf::Vector2u *source, gf::Vector2u *target, bool inversed)
{
  Movement new_movement;

  new_movement.source = get_pos_from_vector(source, inversed);
  new_movement.target = get_pos_from_vector(target, inversed);

  return new_movement;
}
