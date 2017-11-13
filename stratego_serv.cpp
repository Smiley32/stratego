#include <iostream>
#include <string>
#include "s_grid.h"
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char *argv[])
{
  boost::system::error_code error;
  int tmp = 0;
  bool tmp_b = true;

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

    // Envoi du message d'acceptation au client
    boost::system::error_code ignored_error;

    void *fullData = malloc(sizeof(int) + sizeof(bool));
    memcpy(fullData, &tmp, sizeof(int));
    memcpy(&fullData + sizeof(int), &tmp_b, sizeof(bool));
    boost::asio::write(first_client, boost::asio::buffer(fullData, sizeof(fullData)), boost::asio::transfer_all(), ignored_error);

    // Attente du second client
    tcp::socket second_client(io_service);
    acceptor.accept(second_client);

    // Envoi d'un message d'acceptation au client
    boost::asio::write(second_client, boost::asio::buffer(fullData, sizeof(fullData)), boost::asio::transfer_all(), ignored_error);


    // Commemencement boucle de lecture des pièces
    boost::array<char, 128> buf;
    s_grid our_grid;
    int piece_pos;
    int piece_value;
    size_t len;

    our_grid.create_empty_grid();

    // first_client == Blue
    // second_client == Red

    for(;;)
    {

      // Lecture de l'ID du message
      /*
      len = boost::asio::read(first_client, boost::asio::buffer(int_buf), boost::asio::transfer_exactly(sizeof(int)), error);
      if (error)
      {
        throw boost::system::system_error(error);
      }

      std::string data(buf.begin(), buf.begin() + len);

      if (data[0] == '1')
      {*/
        /*
        // Lecture de la case
        len = boost::asio::read(first_client, boost::asio::buffer(buf), boost::asio::transfer_exactly(8));
        if (error)
        {
          throw boost::system::system_error(error);
        }

        data(buf.begin(), buf.begin() + len);
        piece_pos = std::stoi(data);

        // Lecture de la valeur de la pièce
        len = boost::asio::read(first_client, boost::asio::buffer(buf), boost::asio::transfer_exactly(8));
        if (error)
        {
          throw boost::system::system_error(error);
        }
        std::string data(buf.begin(), buf.begin() + len);
        piece_value = std::stoi(data);

      }*/

    }
  }
  catch(std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
