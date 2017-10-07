#include <iostream>
#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {
  try {
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
    
    // Envoi d'un message au client
    boost::system::error_code ignored_error;
    boost::asio::write(first_client, boost::asio::buffer("Connexion acceptée"), boost::asio::transfer_all(), ignored_error);
    
    // Attente du second client
    tcp::socket second_client(io_service);
    acceptor.accept(second_client);
    
    // Envoi d'un message au client
    boost::asio::write(second_client, boost::asio::buffer("Connexion acceptée"), boost::asio::transfer_all(), ignored_error);

    
  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  
  return 0;
}
