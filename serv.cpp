#include <iostream>
#include <string>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

std::string say_hello() {
  return "Salut !!!\n";
}

int main(int argc, char *argv[]) {
  try {
    boost::asio::io_service io_service;
    
    // Ecoute des connection, sur TCP port 25565 avec IPv4
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 25565));
    
    // Le serveur n'accepte qu'une connection
    for(;;) {
      // Socket pour le client
      tcp::socket socket(io_service);
      acceptor.accept(socket);
      
      // Envoi du message au client
      std::string message = say_hello();
      
      boost::system::error_code ignored_error;
      boost::asio::write(socket, boost::asio::buffer(message), boost::asio::transfer_all(), ignored_error);
    }
  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  
  return 0;
}
