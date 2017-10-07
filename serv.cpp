#include <iostream>
#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// Test de fonction
std::string say_pret() {
  return "Je suis pret, envoie-moi les coordonnées !\n";
}

/**
 * Récupère un message sur un socket
 * Le message doit être plus court que 128 caractères
 *
 * @param tcp::socket *socket Le socket à utiliser
 * @return std::string Le message lu
 */

 // Récupération d'un message du serveur
std::string get_message(tcp::socket *socket) {
  // Tableau qui va stocket le message
  boost::array<char, 128> buf;

  // Le code d'erreur potentiel
  boost::system::error_code error;

  // len : taille lue
  size_t len = socket->read_some(boost::asio::buffer(buf), error);

  // En cas d'erreur
  if(error) {
    throw boost::system::system_error(error);
  }

  // Conversion du tableau en chaine
  std::string data(buf.begin(), buf.begin() + len);
  return data;
}

int main(int argc, char *argv[]) {
  try {
    boost::asio::io_service io_service;

    // Ecoute des connection, sur TCP port 25565 avec IPv4
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 25565));

    // Le serveur n'accepte qu'une connection
    for(;;) {
      // Socket pour le 1er client (celui qui envoie les infos)
      tcp::socket socket1(io_service);
      acceptor.accept(socket1);

      // Socket pour le 2eme client
      tcp::socket socket2(io_service);
      acceptor.accept(socket2);

      // On attend un message du premier socket (moins de 128 char)
      for(;;) {
        std::string msg = get_message(&socket1);

        std::cout << msg << std::endl;

        // Envoi du message au client
        boost::system::error_code ignored_error;
        boost::asio::write(socket2, boost::asio::buffer(msg), boost::asio::transfer_all(), ignored_error);
      }

      // Fin de l'attente de co
      break;
    }

  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
