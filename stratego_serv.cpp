#include <iostream>
#include <string>
#include "s_grid.h"
#include <time.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <thread>
#include <gf/Queue.h>

#include "packet.h"
#include "message.h"

using boost::asio::ip::tcp;

void player_thread(tcp::socket *socket, tcp::socket *other, gf::Queue<Message> *messages) {
  gf::Queue<Message> tmp;

  bool error = false;
  while( !error ) {
    error = !get_message(*socket, tmp);

    Message msg;
    while(tmp.poll(msg)) {
      printf("polled...\n");
      if(msg.id != ID_message::Text) {
        messages->push(msg);
      } else {
        send_message(*other, msg);
      }
    }
  }
}

int main(int argc, char *argv[])
{
  Packet p;
  boost::asio::io_service io_service;
  boost::system::error_code ignored_error;
  boost::system::error_code error;
  tcp::socket first_client(io_service);
  tcp::socket second_client(io_service);

  tcp::socket *first_client_pointer = &first_client;
  tcp::socket *second_client_pointer = &second_client;

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

  // Files de messages
  gf::Queue<Message> *first_messages = new gf::Queue<Message>();
  gf::Queue<Message> *second_messages = new gf::Queue<Message>();

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

    if((rand()%2) == 1) {
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

    // Création des processus de réception
    try {
      std::thread first_thread(player_thread, &first_client, &second_client, first_messages);
      first_thread.detach();
      std::thread second_thread(player_thread, &second_client, &first_client, second_messages);
      second_thread.detach();
    } catch(std::exception &e) {
      exit(-1);
    }
    
    our_grid.create_empty_grid();

    // Boucle d'acceptation des pièces
    bool inversed = true;
    while(!our_grid.start_game()) {
      bool msg_lu = false;
      while(!msg_lu) {
        msg_lu = first_messages->poll(new_message);
        if(!msg_lu) {
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
      }

      if(new_message.id != ID_message::Initiate) {
        if(new_message.id == ID_message::Quit || new_message.id == ID_message::Error) {
          gf::Log::error("\nThe client has quited or crashed\n");
          new_message = create_end_message(true);
          send_message(*second_client_pointer, new_message);
          new_message = create_end_message(false);
          send_message(*first_client_pointer, new_message);

          exit(-1);
        }

        new_message = create_accept_message(false);
        send_message(*first_client_pointer, new_message);

        gf::Log::error("\nSignal Error: Expected 1 but get %d\n", (int) new_message.id);
        continue;
      }

      // Verif du message
      for(size_t i = 0; i < PLAYER_MAX_PIECES; i++) {
        first_p_pos = new_message.data.initiate.pieces[i].pos;
        first_p_value = new_message.data.initiate.pieces[i].value;

        get_vector_coord(&first_coo2D, first_p_pos, inversed);
        current_piece.rank = (Rank) first_p_value;
        current_piece.side = inversed ? Side::Red : Side::Blue;

        if(!our_grid.create_piece(first_coo2D, current_piece)) {
          new_message = create_accept_message(false);
          send_message(*first_client_pointer, new_message);
          break;
        }
      }

      if(inversed && our_grid.red_t_ok()) {
        new_message = create_accept_message(true);
        send_message(*first_client_pointer, new_message);

        // inversion des deux clients (les pointeurs seulement pour ne pas gener les threads de reception)
        std::swap(first_client_pointer, second_client_pointer);
        gf::Log::info("swap\n");
        std::swap(first_messages, second_messages);
        inversed = !inversed;
      } else if(!inversed && our_grid.blue_t_ok()) {
        new_message = create_accept_message(true);
        send_message(*first_client_pointer, new_message);

        // devrait être fini
        // --> On réinverse pour la suite
        std::swap(first_client_pointer, second_client_pointer);
        gf::Log::info("swap\n");
        std::swap(first_messages, second_messages);
        inversed = !inversed;
      }
    }

    // SIGNAL LANCEMENT DU JEU
    gf::Log::info("\nSignal 2 for start sent to both client\n");

    new_message = create_play_message();
    send_message(*first_client_pointer, new_message);
    send_message(*second_client_pointer, new_message);

    while (!our_grid.game_is_end()) {
      // ACTION PREMIER JOUEUR
      gf::Log::info("\nSignal 2 for play sent to first client\n");
      new_message = create_play_message();
      send_message(*first_client_pointer, new_message);

      accepted = false;
      while(!accepted) {
        bool msg_lu = false;
        while(!msg_lu) {
          msg_lu = first_messages->poll(new_message);
          if(!msg_lu) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
          }
        }

        if(new_message.id != ID_message::Move) {
          if(new_message.id == ID_message::Quit || new_message.id == ID_message::Error) {
            gf::Log::error("\nThe client has quited or crashed\n");
            new_message = create_end_message(true);
            send_message(*second_client_pointer, new_message);
            new_message = create_end_message(false);
            send_message(*first_client_pointer, new_message);

            exit(-1);
          }
          gf::Log::error("\nSignal Error: Expected signal Move (3) but get %c\n", buf[0]);
          continue;
        }

        first_p_pos = new_message.data.move.source;
        second_p_pos = new_message.data.move.target;

        get_vector_coord(&first_coo2D, first_p_pos, inversed);
        get_vector_coord(&second_coo2D, second_p_pos, inversed);
        first_p_value = our_grid.get_value(first_coo2D);
        second_p_value = our_grid.get_value(second_coo2D);

        gf::Log::info("\nAsk for a moove from %d %d (team 1) to %d %d, piece have value %d, target is %d\n", first_coo2D.x, first_coo2D.y, second_coo2D.x, second_coo2D.y, first_p_value, second_p_value);

        if((inversed && our_grid.get_side(first_coo2D) == Side::Red) || (!inversed && our_grid.get_side(first_coo2D) == Side::Blue)) {
          std::cout << (inversed ? "Red" : "Blue") << std::endl;
          accepted = our_grid.move_piece(first_coo2D, second_coo2D);
        } else {
          /*if(our_grid.get_side(first_coo2D) == Side::Blue) {
            std::cout << "Blue" << std::endl;
          }
          else
          {
            std::cout << "Other" << std::endl;
          }*/
          gf::Log::info("\nYou can't moove a piece from the enemy team\n");
        }

        new_message = create_accept_message(accepted);
        send_message(*first_client_pointer, new_message);
      }

      // Envoie update premier client
      gf::Log::info("\nSignal 4 for update sent to first client\n");
      new_movement = create_movement(&first_coo2D, &second_coo2D, !inversed);

      if(our_grid.had_collision()) {
        if((int) Rank::Empty == our_grid.get_value(first_coo2D) && (int) Rank::Empty == our_grid.get_value(second_coo2D)) {
          new_result = Result::Draw;
        } else {
          if(second_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == (int) Rank::Empty) {
            new_result = Result::Lose;
          } else {
            new_result = Result::Win;
          }
        }

        new_message = create_update_message(new_movement, second_p_value, new_result);
      } else {
        new_message = create_update_message(new_movement);
      }

      send_message(*first_client_pointer, new_message);

      // Envoi update deuxième client
      gf::Log::info("\nSignal 4 for update sent to second client\n");
      new_movement = create_movement(&first_coo2D, &second_coo2D, inversed);
      if(our_grid.had_collision()) {
        if((int) Rank::Empty == our_grid.get_value(first_coo2D) && (int) Rank::Empty == our_grid.get_value(second_coo2D)) {
          new_result = Result::Draw;
        } else {
          if(second_p_value == our_grid.get_value(second_coo2D) && our_grid.get_value(first_coo2D) == (int) Rank::Empty) {
            new_result = Result::Win;
          } else {
            new_result = Result::Lose;
          }
        }

        new_message = create_update_message(new_movement, first_p_value, new_result);
      } else {
        new_message = create_update_message(new_movement);
      }

      send_message(*second_client_pointer, new_message);

      if(our_grid.game_is_end()) {
        gf::Log::info("\nThe first client won !\n");
        new_message = create_end_message(true);

        send_message(*first_client_pointer, new_message);

        gf::Log::info("\nThe second client lost !\n");
        new_message = create_end_message(false);

        send_message(*second_client_pointer, new_message);
      }

      // Inversion des clients
      inversed = !inversed;
      std::swap(first_client_pointer, second_client_pointer);
      gf::Log::info("swap\n");
      std::swap(first_messages, second_messages);
    }
  }
  catch(std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
