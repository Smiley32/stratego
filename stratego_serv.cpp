#include <iostream>
#include <string>
#include "s_grid.h"
#include <time.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "packet.h"
#include "message.h"

using boost::asio::ip::tcp;

boost::array<char, 128> get_message_serv(tcp::socket *socket)
{
  boost::array<char, 128> buf;
  boost::system::error_code error;
  bool is_ok = false;
  size_t len;


    len = socket->read_some(boost::asio::buffer(buf), error);

    if (error)
    {
      throw boost::system::system_error(error);
    }



    gf::Log::info("\nMessage reçu:\n");
    for (size_t i = 0; i < len; i++)
    {
      std::cout << (int) buf[i] << ' ';
    }

    std::cout << std::endl;

  // std::string data(buf.begin(), buf.begin() + len);
  return buf;
}

int main(int argc, char *argv[])
{
  Packet p;
  boost::asio::io_service io_service;
  boost::system::error_code ignored_error;
  boost::system::error_code error;
  tcp::socket first_client(io_service);
  tcp::socket second_client(io_service);
  Message new_message;
  Movement new_movement;
  Result new_result;

  boost::array<char, 128> buf;
  s_grid our_grid;
  int port;
  int first_p_pos;
  int second_p_pos;
  int piece_value;
  int first_p_value;
  int second_p_value;
  bool red_team_rdy = false;
  bool blue_team_rdy = false;
  bool accepted;
  Piece current_piece;
  gf::Vector2u first_coo2D;
  gf::Vector2u second_coo2D;
  size_t len;
  srand(time(NULL));


  try
  {
    if (argc != 2)
    {
      gf::Log::error("\nError: Invalid number of arguments\n");
      exit(1);
    }
    port = atoi(argv[1]);
    if(port <= 0) {
      // Port incorrect, on met une valeur par défaut
      port = 25565;
    }

    std::cout << "Port utilisé : " << port << std::endl;

    // Ecoute des connections
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));

    if ((rand()%2) == 1)
    {
      // Attente du premier client
      gf::Log::info("\nWaiting for the first client\n");

      acceptor.accept(first_client);

      gf::Log::info("\nFirst client connection\n");

      new_message = create_accept_message(true);
      send_message(first_client, new_message);

      // Attente du second client
      gf::Log::info("\nWaiting for the second client\n");

      acceptor.accept(second_client);

      gf::Log::info("\nSecond client connection\n");

      send_message(second_client, new_message);
    }
    else
    {
      // Attente du second client
      gf::Log::info("\nWaiting for the second client\n");

      acceptor.accept(second_client);

      gf::Log::info("\nSecond client connection\n");

      new_message = create_accept_message(true);
      send_message(second_client, new_message);

      // Attente du premier client
      gf::Log::info("\nWaiting for the first client\n");

      acceptor.accept(first_client);

      send_message(first_client, new_message);
    }

    // Commemencement boucle de lecture des pièces

    our_grid.create_empty_grid();

    // BOUCLE ACCEPTATION DES PIECES
    while (!our_grid.start_game())
    {
      // SI TEAM ROUGE PAS ENCORE PRETE (premier client)
      if (!red_team_rdy)
      {
        new_message = get_message(first_client);

        if (new_message.id != ID_message::Initiate)
        {
          new_message = create_accept_message(false);
          send_message(first_client, new_message);

          gf::Log::error("\nSignal Error: Expected 1 but get %d\n", (int) new_message.id);
          continue;
        }

        for (size_t i = 0; i < PLAYER_MAX_PIECES; i++)
        {
          first_p_pos = new_message.data.initiate.pieces[i].pos;
          first_p_value = new_message.data.initiate.pieces[i].value;

          get_vector_coord(&first_coo2D, first_p_pos, true);
          current_piece.rank = (Rank) first_p_value;
          current_piece.side = Side::Red;

          if (!our_grid.create_piece(first_coo2D, current_piece))
          {
            new_message = create_accept_message(false);
            send_message(first_client, new_message);
            break;
          }
        }
        red_team_rdy = our_grid.red_t_ok();

        if (red_team_rdy)
        {
          new_message = create_accept_message(true);
          send_message(first_client, new_message);
        }
      }

      // SI TEAM BLEUE PAS ENCORE PRETE
      if (!blue_team_rdy)
      {
        new_message = get_message(second_client);

        if (new_message.id != ID_message::Initiate)
        {
          new_message = create_accept_message(false);
          send_message(second_client, new_message);

          gf::Log::error("\nSignal Error: Expected 1 but get %d\n", (int) new_message.id);
          continue;
        }

        for (size_t i = 0; i < PLAYER_MAX_PIECES; i++)
        {
          first_p_pos = new_message.data.initiate.pieces[i].pos;
          first_p_value = new_message.data.initiate.pieces[i].value;

          get_vector_coord(&first_coo2D, first_p_pos, true);
          current_piece.rank = (Rank) first_p_value;
          current_piece.side = Side::Blue;

          if (!our_grid.create_piece(first_coo2D, current_piece))
          {
            new_message = create_accept_message(false);
            send_message(second_client, new_message);
            break;
          }
        }
        blue_team_rdy = our_grid.blue_t_ok();

        if(blue_team_rdy) {
          new_message = create_accept_message(true);
          send_message(second_client, new_message);
        }
      }
    }

    // SIGNAL LANCEMENT DU JEU
    gf::Log::info("\nSignal 2 for start sent to both client\n");

    new_message = create_play_message();
    send_message(first_client, new_message);
    send_message(second_client, new_message);

    while (!our_grid.game_is_end())
    {
      // ACTION PREMIER JOUEUR
      gf::Log::info("\nSignal 2 for play sent to first client\n");
      new_message = create_play_message();
      send_message(first_client, new_message);

      accepted = false;
      while (!accepted)
      {
        new_message = get_message(first_client);

        if (new_message.id != ID_message::Move)
        {
          gf::Log::error("\nSignal Error: Expected signal Move (3) but get %c\n", buf[0]);
          continue;
        }

        first_p_pos = new_message.data.move.source;
        second_p_pos = new_message.data.move.target;

        get_vector_coord(&first_coo2D, first_p_pos, true);
        get_vector_coord(&second_coo2D, second_p_pos, true);
        first_p_value = our_grid.get_value(first_coo2D);
        second_p_value = our_grid.get_value(second_coo2D);

        gf::Log::info("\nAsk for a moove from %d %d (team 1) to %d %d, piece have value %d, target is %d\n", first_coo2D.x, first_coo2D.y, second_coo2D.x, second_coo2D.y, first_p_value, second_p_value);

        if (our_grid.get_side(first_coo2D) == Side::Red)
        {
          std::cout << "Red" << std::endl;
          accepted = our_grid.move_piece(first_coo2D, second_coo2D);
        }
        else
        {
          if (our_grid.get_side(first_coo2D) == Side::Blue)
          {
            std::cout << "Blue" << std::endl;
          }
          else
          {
            std::cout << "Other" << std::endl;
          }
          gf::Log::info("\nYou can't moove a piece from the enemy team\n");
        }

        if (accepted)
        {
          new_message = create_accept_message(true);
          send_message(second_client, new_message);
        }
        else
        {
          new_message = create_accept_message(false);
          send_message(second_client, new_message);
        }
      }

      // Envoie update premier client
      gf::Log::info("\nSignal 4 for update sent to first client\n");
      new_movement = create_movement(&first_coo2D, &second_coo2D, false);

      if (our_grid.had_collision())
      {
        if ((int) Rank::Empty == our_grid.get_value(first_coo2D) && (int) Rank::Empty == our_grid.get_value(second_coo2D))
        {
          new_result = Result::Draw;
        }
        else
        {
          if (second_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == (int) Rank::Empty)
          {
            new_result = Result::Lose;
          }
          else
          {
            new_result = Result::Win;
          }
        }

        new_message = create_update_message(new_movement, second_p_value, new_result);
      }
      else
      {
        new_message = create_update_message(new_movement);
      }

      send_message(first_client, new_message);

      // Envoi update deuxième client
      gf::Log::info("\nSignal 4 for update sent to second client\n");
      new_movement = create_movement(&first_coo2D, &second_coo2D, true);
      if (our_grid.had_collision())
      {
        if ((int) Rank::Empty == our_grid.get_value(first_coo2D) && (int) Rank::Empty == our_grid.get_value(second_coo2D))
        {
          new_result = Result::Draw;
        }
        else
        {
          if (second_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == (int) Rank::Empty)
          {
            new_result = Result::Lose;
          }
          else
          {
            new_result = Result::Win;
          }
        }

        new_message = create_update_message(new_movement, first_p_value, new_result);
      }
      else
      {
        new_message = create_update_message(new_movement);
      }

      send_message(second_client, new_message);

      if (our_grid.game_is_end())
      {
        gf::Log::info("\nThe first client won !\n");
        new_message = create_end_message(true);

        send_message(first_client, new_message);

        gf::Log::info("\nThe second client lost !\n");
        new_message = create_end_message(false);

        send_message(second_client, new_message);
      }

      // ACTION DEUXIEME JOUEUR
      gf::Log::info("\nSignal 2 for play sent to second client\n");
      new_message = create_play_message();
      send_message(second_client, new_message);

      accepted = false;
      while (!accepted)
      {
        new_message = get_message(second_client);

        if (new_message.id != ID_message::Move)
        {
          gf::Log::error("\nSignal Error: Expected signal Move (3) but get %c\n", buf[0]);
          continue;
        }

        first_p_pos = new_message.data.move.source;
        second_p_pos = new_message.data.move.target;

        get_vector_coord(&first_coo2D, first_p_pos, false);
        get_vector_coord(&second_coo2D, second_p_pos, false);
        first_p_value = our_grid.get_value(first_coo2D);
        second_p_value = our_grid.get_value(second_coo2D);
        gf::Log::info("\nAsk for a moove from %d %d (team 2) to %d %d (team 1), piece have value %d, target is %d\n", first_coo2D.x, first_coo2D.y, second_coo2D.x, second_coo2D.y, first_p_value, second_p_value);

        if (our_grid.get_side(first_coo2D) == Side::Blue)
        {
          std::cout << "Blue" << std::endl;
          accepted = our_grid.move_piece(first_coo2D, second_coo2D);
        }
        else
        {
          if (our_grid.get_side(first_coo2D) == Side::Red)
          {
            std::cout << "Red" << std::endl;
          }
          else
          {
            std::cout << "Other" << std::endl;
          }
        }

        if (accepted)
        {
          new_message = create_accept_message(true);
          send_message(second_client, new_message);
        }
        else
        {
          new_message = create_accept_message(false);
          send_message(second_client, new_message);
        }
      }

      // Envoie update premier client
      gf::Log::info("\nSignal 4 for update sent to first client\n");

      new_movement = create_movement(&first_coo2D, &second_coo2D, false);

      if (our_grid.had_collision())
      {
        if ((int) Rank::Empty == our_grid.get_value(first_coo2D) && (int) Rank::Empty == our_grid.get_value(second_coo2D))
        {
          new_result = Result::Draw;
        }
        else
        {
          if (first_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == (int) Rank::Empty)
          {
            new_result = Result::Lose;
          }
          else
          {
            new_result = Result::Win;
          }
        }

        new_message = create_update_message(new_movement, first_p_value, new_result);
      }
      else
      {
        new_message = create_update_message(new_movement);
      }

      send_message(first_client, new_message);


      // Envoi update deuxième client
      gf::Log::info("\nSignal 4 for update sent to second client\n");

      new_movement = create_movement(&first_coo2D, &second_coo2D, true);

      if (our_grid.had_collision())
      {
        if ((int) Rank::Empty == our_grid.get_value(first_coo2D) && (int) Rank::Empty == our_grid.get_value(second_coo2D))
        {
          new_result = Result::Draw;
        }
        else
        {
          if (second_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == (int) Rank::Empty)
          {
            new_result = Result::Lose;
          }
          else
          {
            new_result = Result::Win;
          }
        }

        new_message = create_update_message(new_movement, second_p_value, new_result);
      }
      else
      {
        new_message = create_update_message(new_movement);
      }

      send_message(second_client, new_message);

      if (our_grid.game_is_end())
      {
        gf::Log::info("\nThe second client won !\n");
        new_message = create_end_message(true);
        send_message(second_client, new_message);
        gf::Log::info("\nThe first client loose !\n");
        new_message = create_end_message(false);
        send_message(second_client, new_message);
      }
    }
  }
  catch(std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
