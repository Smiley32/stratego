#include <iostream>
#include <string>
#include "s_grid.h"
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "packet.h"

using boost::asio::ip::tcp;

boost::array<char, 128> get_message(tcp::socket *socket)
{
  boost::array<char, 128> buf;
  boost::system::error_code error;

  size_t len = socket->read_some(boost::asio::buffer(buf), error);

  if (error)
  {
    throw boost::system::system_error(error);
  }

  // std::string data(buf.begin(), buf.begin() + len);
  return buf;
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

int main(int argc, char *argv[])
{
  boost::system::error_code error;

  try
  {
    boost::asio::io_service io_service;

    int port;

    // Demande du port à utiliser
    std::cout << "Quel port souhaitez-vous utiliser ?" << std::endl;
    std::cin >> port;
    if(port <= 0) {
      // Port incorrect, on met une valeur par défaut
      port = 25565;
    }

    std::cout << "Port utilisé : " << port << std::endl;

    // Ecoute des connections
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));

    // Attente du premier client
    tcp::socket first_client(io_service);
    acceptor.accept(first_client);

    boost::system::error_code ignored_error;

    Packet p;
    p.append(0);
    p.append(1);

    // Envoi du message d'acceptation au client
    boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
    std::cout << "fait" << std::endl;

    // Attente du second client
    tcp::socket second_client(io_service);
    acceptor.accept(second_client);

    // Envoi d'un message d'acceptation au client
    boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);

    p.clear();
    // Commemencement boucle de lecture des pièces
    boost::array<char, 128> buf;
    s_grid our_grid;
    int piece_pos;
    int spiece_pos;
    int piece_value;
    bool r_rdy = false;
    bool b_rdy = false;
    bool accepted;
    Piece current_piece;
    gf::Vector2u coo2D;
    gf::Vector2u scoo2D;
    size_t len;

    our_grid.create_empty_grid();

    // first_client == Blue
    // second_client == Red

    // BOUCLE ACCEPTATION DES PIECES
    while (!our_grid.start_game())
    {
      // SI TEAM ROUGE PAS ENCORE PRETE
      if (!r_rdy)
      {
        buf = get_message(&first_client);

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
          piece_pos = (int) (buf[i]);
          piece_value = (int) (buf[i+1]);

          get_vector_coord(&coo2D, piece_pos, true);
          current_piece.rank = (Rank) piece_value;
          current_piece.side = Side::Red;

          if (!our_grid.create_piece(coo2D, current_piece))
          {
            p.append(0);
            p.append(0);
            boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
            p.clear();
            break;
          }
        }
        r_rdy = our_grid.red_t_ok();

        if(r_rdy) {
          p.append(0);
          p.append(1);
          boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
        }
      }

      // SI TEAM BLEUE PAS ENCORE PRETE
      if (!b_rdy)
      {
        buf = get_message(&second_client);

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
          piece_pos = (int) (buf[i]);
          piece_value = (int) (buf[i+1]);

          get_vector_coord(&coo2D, piece_pos, false);
          current_piece.rank = (Rank) piece_value;
          current_piece.side = Side::Blue;

          if (!our_grid.create_piece(coo2D, current_piece))
          {
            p.append(0);
            p.append(0);
            boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
            p.clear();
            break;
          }
        }
        b_rdy = our_grid.blue_t_ok();

        if(b_rdy) {
          p.append(0);
          p.append(1);
          boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
        }
      }
    }

    // SIGNAL LANCEMENT DU JEU
    p.append(2);
    boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
    boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
    p.clear();

    while (our_grid.game_is_end())
    {
      // ACTION PREMIER JOUEUR
      p.append(2);
      boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
      p.clear();

      accepted = false;
      while (accepted)
      {
        buf = get_message(&first_client);

        if (buf[0] != 3)
        {
          p.append(0);
          p.append(0);
          boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
          gf::Log::error("\nSignal Error: Expected 3 but get %c\n", buf[0]);
        }

        piece_pos = (int) (buf[1]);
        spiece_pos = (int) (buf[2]);

        get_vector_coord(&coo2D, piece_pos, true);
        get_vector_coord(&scoo2D, spiece_pos, true);
        accepted = our_grid.move_piece(coo2D, scoo2D);
      }

      // TODO MAJ pour les deux clients

      if (our_grid.game_is_end())
      {
        p.append(5);
        p.append(1);
        boost::asio::write(first_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
        p.clear();
        p.append(5);
        p.append(0);
        boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
        p.clear();
      }

      // ACTION DEUXIEME JOUEUR
      p.append(2);
      boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
      p.clear();

      accepted = false;
      while (accepted)
      {
        buf = get_message(&second_client);

        if (buf[0] != 3)
        {
          p.append(0);
          p.append(0);
          boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
          p.clear();
          gf::Log::error("\nSignal Error: Expected 3 but get %c\n", buf[0]);
        }

        piece_pos = (int) (buf[1]);
        spiece_pos = (int) (buf[2]);

        get_vector_coord(&coo2D, piece_pos, false);
        get_vector_coord(&scoo2D, spiece_pos, false);
        accepted = our_grid.move_piece(coo2D, scoo2D);
      }

      // TODO MAJ pour les deux clients

      if (our_grid.game_is_end())
      {
        p.append(5);
        p.append(1);
        boost::asio::write(second_client, boost::asio::buffer(p.getData(), p.getDataSize()), boost::asio::transfer_all(), ignored_error);
        p.clear();
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
