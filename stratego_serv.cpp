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

      p.append(0);
      p.append(1);
      gf::Log::info("\n\t0_1\n");

      // Envoi du message d'acceptation au client
      boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);

      // Attente du second client
      gf::Log::info("\nWaiting for the second client\n");

      acceptor.accept(second_client);

      gf::Log::info("\nSecond client connection\n");

      // Envoi d'un message d'acceptation au client
      boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);

      p.clear();
      gf::Log::info("\n\t0_1\n");
    }
    else
    {
      // Attente du second client
      gf::Log::info("\nWaiting for the second client\n");

      acceptor.accept(second_client);

      gf::Log::info("\nSecond client connection\n");

      p.append(0);
      p.append(1);

      // Envoi d'un message d'acceptation au client
      boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);

      gf::Log::info("\n\t0_1\n");

      // Attente du premier client
      gf::Log::info("\nWaiting for the first client\n");

      acceptor.accept(first_client);

      gf::Log::info("\nFirst client connection\n");


      gf::Log::info("\n\t0_1\n");

      // Envoi du message d'acceptation au client
      boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);

      p.clear();
    }

    // Commemencement boucle de lecture des pièces

    our_grid.create_empty_grid();

    // first_client == Blue
    // second_client == Red

    // BOUCLE ACCEPTATION DES PIECES
    while (!our_grid.start_game())
    {
      // SI TEAM ROUGE PAS ENCORE PRETE
      if (!red_team_rdy)
      {
        buf = get_message_serv(&first_client);

        if (buf[0] != 1)
        {
          p.append(0); // Id du message (acceptation)
          p.append(0); // 0 -> false ; 1 -> true
          boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);

          gf::Log::error("\nSignal Error: Expected 1 but get %c\n", buf[0]);
          p.clear();
          continue;
        }

        for (size_t i = 1; i < 81; i = i + 2)
        {
          first_p_pos = (int) (buf[i]);
          first_p_value = (int) (buf[i+1]);

          get_vector_coord(&first_coo2D, first_p_pos, true);
          current_piece.rank = (Rank) first_p_value;
          current_piece.side = Side::Red;

          if (!our_grid.create_piece(first_coo2D, current_piece))
          {
            p.append(0);
            p.append(0);
            boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
            p.clear();
            break;
          }
        }
        red_team_rdy = our_grid.red_t_ok();

        if (red_team_rdy)
        {
          p.append(0);
          p.append(1);
          boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
        }
      }

      // SI TEAM BLEUE PAS ENCORE PRETE
      if (!blue_team_rdy)
      {
        buf = get_message_serv(&second_client);

        if (buf[0] != 1)
        {
          p.append(0);
          p.append(0);
          boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          gf::Log::error("\nSignal Error: Expected 1 but get %c\n", buf[0]);
          p.clear();
          continue;
        }

        for (size_t i = 1; i < 81; i = i + 2)
        {
          first_p_pos = (int) (buf[i]);
          first_p_value = (int) (buf[i+1]);

          get_vector_coord(&first_coo2D, first_p_pos, false);
          current_piece.rank = (Rank) first_p_value;
          current_piece.side = Side::Blue;

          if (!our_grid.create_piece(first_coo2D, current_piece))
          {
            p.append(0);
            p.append(0);
            boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
            p.clear();
            break;
          }
        }
        blue_team_rdy = our_grid.blue_t_ok();

        if(blue_team_rdy) {
          p.append(0);
          p.append(1);
          boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
        }
      }
    }

    // SIGNAL LANCEMENT DU JEU
    gf::Log::info("\nSignal 2 for start sent to both client\n");
    p.append(2);
    boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
    boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
    p.clear();

    while (!our_grid.game_is_end())
    {
      // ACTION PREMIER JOUEUR
      gf::Log::info("\nSignal 2 for play sent to first client\n");
      p.append(2);
      boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
      p.clear();

      accepted = false;
      while (!accepted)
      {
        buf = get_message_serv(&first_client);

        if (buf[0] != 3)
        {/*
          p.append(0);
          p.append(0);
          boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();*/
          gf::Log::error("\nSignal Error: Expected 3 but get %c\n", buf[0]);
          continue;
        }

        first_p_pos = (int) (buf[1]);
        second_p_pos = (int) (buf[2]);

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
          p.append(0);
          p.append(1);
          boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
        }
        else
        {
          p.append(0);
          p.append(0);
          boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
        }
      }

      // Envoie update premier client
      p.append(4);
      gf::Log::info("\nSignal 4 for update sent to first client\n");

      if (our_grid.had_collision())
      {
        p.append(1);
        p.append(get_pos_from_vector(&first_coo2D, false));
        p.append(get_pos_from_vector(&second_coo2D, false));
        p.append(second_p_value);
        gf::Log::info("\n\t4_1_%d_%d_%d", get_pos_from_vector(&first_coo2D, true), get_pos_from_vector(&second_coo2D, true), second_p_value);

        if (13 == our_grid.get_value(first_coo2D) && 13 == our_grid.get_value(second_coo2D))
        {
          p.append(2);
          std::cout << "_2" << std::endl;
        }
        else
        {
          if (second_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == 13)
          {
            p.append(0);
            std::cout << "_0" << std::endl;
          }
          else
          {
            p.append(1);
            std::cout << "_1" << std::endl;
          }
        }
      }
      else
      {
        p.append(0);
        p.append(get_pos_from_vector(&first_coo2D, false));
        p.append(get_pos_from_vector(&second_coo2D, false));
        gf::Log::info("\n\t4_0_%d_%d\n", get_pos_from_vector(&first_coo2D, false), get_pos_from_vector(&second_coo2D, false));
      }

      boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
      p.clear();

      // Envoi update deuxième client
      gf::Log::info("\nSignal 4 for update sent to second client\n");
      p.append(4);

      if (our_grid.had_collision())
      {
        p.append(1);
        p.append(get_pos_from_vector(&first_coo2D, true));
        p.append(get_pos_from_vector(&second_coo2D, true));
        p.append(first_p_value);
        gf::Log::info("\n\t4_1_%d_%d_%d", get_pos_from_vector(&first_coo2D, true),get_pos_from_vector(&second_coo2D, true), first_p_value);

        if (13 == our_grid.get_value(first_coo2D) && 13 == our_grid.get_value(second_coo2D))
        {
          p.append(2);
          std::cout << "_2" << std::endl;
        }
        else
        {
          if (second_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == 13)
          {
            p.append(1);
            std::cout << "_1" << std::endl;
          }
          else
          {
            p.append(0);
            std::cout << "_0" << std::endl;
          }
        }
      }
      else
      {
        p.append(0);
        p.append(get_pos_from_vector(&first_coo2D, true));
        p.append(get_pos_from_vector(&second_coo2D, true));
        gf::Log::info("\n\t4_0_%d_%d\n", get_pos_from_vector(&first_coo2D, true), get_pos_from_vector(&second_coo2D, true));
      }

      boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
      p.clear();

      if (our_grid.game_is_end())
      {
        gf::Log::info("\nThe first client won !\n");
        gf::Log::info("\n\t5_1\n");
        // Envoi signal de fin premier client
        p.append(5);
        p.append(1);
        boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
        p.clear();

        gf::Log::info("\nThe second client lost !\n");
        gf::Log::info("\n\t5_0\n");
        // Envoi signal de fin deuxième client
        p.append(5);
        p.append(0);
        boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
        p.clear();
      }

      // ACTION DEUXIEME JOUEUR
      gf::Log::info("\nSignal 2 for play sent to second client\n");
      p.append(2);
      boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
      p.clear();

      accepted = false;
      while (!accepted)
      {
        buf = get_message_serv(&second_client);

        if (buf[0] != 3)
        {/*
          p.append(0);
          p.append(0);
          boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();*/
          gf::Log::error("\nSignal Error: Expected 3 but get %c\n", buf[0]);
          continue;
        }

        first_p_pos = (int) (buf[1]);
        second_p_pos = (int) (buf[2]);
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
          p.append(0);
          p.append(1);
          boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
        }
        else
        {
          p.append(0);
          p.append(0);
          boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
        }
      }

      // Envoie update premier client
      gf::Log::info("\nSignal 4 for update sent to first client\n");
      p.append(4);

      if (our_grid.had_collision())
      {
        gf::Log::info("\n\t4_1_%d_%d_%d", get_pos_from_vector(&first_coo2D, false),get_pos_from_vector(&second_coo2D, false), second_p_value);
        p.append(1);
        p.append(get_pos_from_vector(&first_coo2D, false));
        p.append(get_pos_from_vector(&second_coo2D, false));
        p.append(first_p_value);

        if (13 == our_grid.get_value(first_coo2D) && 13 == our_grid.get_value(second_coo2D))
        {
          p.append(2);
          std::cout << "_2" << std::endl;
        }
        else
        {
          if (first_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == 13)
          {
            p.append(0);
            std::cout << "_0" << std::endl;
          }
          else
          {
            p.append(1);
            std::cout << "_1" << std::endl;
          }
        }
      }
      else
      {
        p.append(0);
        p.append(get_pos_from_vector(&first_coo2D, false));
        p.append(get_pos_from_vector(&second_coo2D, false));
        gf::Log::info("\n\t4_0_%d_%d\n", get_pos_from_vector(&first_coo2D, false), get_pos_from_vector(&second_coo2D, false));
      }

      boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
      p.clear();

      // Envoi update deuxième client
      gf::Log::info("\nSignal 4 for update sent to second client\n");
      p.append(4);

      if (our_grid.had_collision())
      {
        gf::Log::info("\n\t4_1_%d_%d_%d", get_pos_from_vector(&first_coo2D, true), get_pos_from_vector(&second_coo2D, true), first_p_value);
        p.append(1);
        p.append(get_pos_from_vector(&first_coo2D, true));
        p.append(get_pos_from_vector(&second_coo2D, true));
        p.append(second_p_value);

        if (13 == our_grid.get_value(first_coo2D) && 13 == our_grid.get_value(second_coo2D))
        {
          p.append(2);
          std::cout << "_2" << std::endl;
        }
        else
        {
          if (second_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == 13)
          {
            p.append(0);
            std::cout << "_0" << std::endl;
          }
          else
          {
            p.append(1);
            std::cout << "_1" << std::endl;
          }
        }
      }
      else
      {
        p.append(0);
        p.append(get_pos_from_vector(&first_coo2D, true));
        p.append(get_pos_from_vector(&second_coo2D, true));
        gf::Log::info("\n\t4_0_%d_%d\n", get_pos_from_vector(&first_coo2D, true), get_pos_from_vector(&second_coo2D, true));
      }

      boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
      p.clear();

      if (our_grid.game_is_end())
      {
        gf::Log::info("\nThe second client won !\n");
        gf::Log::info("\n\t5_1\n");
        p.append(5);
        p.append(1);
        boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
        p.clear();
        gf::Log::info("\nThe first client loose !\n");
        gf::Log::info("\n\t5_0\n");
        p.append(5);
        p.append(0);
        boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
        p.clear();
      }
    }
  }
  catch(std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
