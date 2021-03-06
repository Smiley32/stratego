#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <stdbool.h>
#include <gf/Entity.h>
#include <gf/TileLayer.h>
#include <gf/RenderTarget.h>
#include <gf/Curves.h>
#include <gf/Color.h>
#include <gf/Queue.h>
#include "packet.h"

using boost::asio::ip::tcp;

#define ID_INDEX 0

#define PLAYER_MAX_PIECES 40
#define NB_CASES 100

#define UPDATE_INDEX 1
#define COLLISION 1

enum class ID_message : int {
  Error = -1,
  Accept = 0,
  Initiate = 1,
  Play = 2,
  Move = 3,
  Update = 4,
  End = 5,
  Quit = 6,
  Text = 7,
  Type = 8
};

enum class Result : int {
  Lose = 0,
  Win = 1,
  Draw = 2
};

struct Movement {
  int source;
  int target;
};

struct Resumed_piece {
  int pos;
  int value;
};

struct Initialize {
  Resumed_piece pieces[PLAYER_MAX_PIECES];
};

struct Update {
  bool collision;
  Movement movement;
  int enemy_value;
  Result result;
};

struct Text {
  int length;
  char txt[100];
};

enum class Type : int {
  Default = 0,
  Timed = 1
};

struct Game_type {
  Type game_type;
  int seconds;
};

struct Message
{
  ID_message id;
  union data
  {
    bool accept;
    Initialize initiate;
    // play: rien
    Movement move;
    Update update;
    Result end;
    // quit: rien
    Text text;
    Game_type type;
  };
  data data;
};

/**
 *  Fonction qui envoie un packet via une socket
 *  @param tcp::socket *socket: la socket où envoyer le paquet
 *  @param Packet &p: le paquet a envoyer
 */
void send_packet(tcp::socket* socket, Packet &p);

/**
 *  Fonction qui récupère un message sous forme de chaine de caractère
 *  @param tcp::socket &socket: la socket d'où vient le message
 *  @param size_t &length: variable qui sert à récupérer la taille de ce qui a été récupérer
 *  @return boost::array<char, 128>: la chaine de caractère récupérée
 */
boost::array<char, 128> get_char_message(tcp::socket &socket, size_t &length);

/**
 * Attends un message (ne gère pas la réception de messages concaténés)
 *
 * @param tcp::socket &socket Socket sur laquelle lire le message
 * @return Message    Message retourné
 */
Message get_message(tcp::socket &socket);

/**
 * Attend un message et l'ajoute à la file.
 * 
 * Dans le cas où plusieurs messages arrivent concaténés, les messages sont tous ajoutés à la file
 * 
 * @param tcp::socket &socket Socket sur laquelle lire le message
 * @param gf::Queue<Message> &file File à laquelle ajouter les messages
 * @return bool false si une erreur est survenue
 */
bool get_message(tcp::socket &socket, gf::Queue<Message> &file);

/**
 *  Envoie un message à travers une socket
 *  @param tcp::socket &socket: la socket sur laquelle envoyer le message
 *  @param Message our_message: le message a envoyer
 */
void send_message(tcp::socket &socket, Message our_message);

/**
 * Fonction qui prépare la structure de message d'acceptation en vue de l'envoi
 * Utilisée par le serveur lors de la connection des clients
 * @param bool accepted: Si la connection est acceptée ou non
 * @return Message: la structure prête à l'envoi
 */
Message create_accept_message(bool accepted);

/**
 * Fonction qui prépare la structure de message d'initialisation en vue de l'envoi
 * Utilisée par les clients pour transmettre le placement de leur pièces en début de jeu
 * @param struct Initialize pieces: structure contenant le tableau de toutes les pièces
 * @return Message: la structure prête à l'envoi
 */
Message create_initiate_message(struct Initialize pieces);

/**
 * Fonction qui prépare la structure de message "JOUE" en vue de l'envoi
 * Utilisée par le serveur pour le début de partie et pour le début de tour de chaque client
 * @return Message: la structure prête à l'envoi
 */
Message create_play_message();

/**
 * Fonction qui prépare la structure de message de déplacement en vue de l'envoi
 * Utilisée par les clients pour le signalement de leur mouvements
 * @param struct Movement movement: structure contenant les pos d'origine et de destination
 * @return Message: la structure prête à l'envoi
 */
Message create_move_message(struct Movement movement);

/**
 * Fonction qui prépare la structure de message d'update en vue de l'envoi (Sans collision)
 * Utilisée par le serveur pour informer de la mise à jour du jeu
 * @param struct Movement movement: Le déplacement (origine, destination)
 * @return Message: la structure prête à l'envoi
 */
Message create_update_message(struct Movement movement);

/**
 * Fonction qui prépare la structure de message d'update en vue de l'envoi (Avec collision)
 * Utilisée par le serveur pour informer de la mise à jour du jeu
 * @param struct Movement movement: Le déplacement (origine, destination)
 * @param int enemy_value: la valeur ennemie
 * @param Result result: le résultat du déplacement
 * @return Message: la structure prête à l'envoi
 */
Message create_update_message(struct Movement movement, int enemy_value, Result result);

/**
 * Fonction qui prépare la structure de message de fin de jeu en vue de l'envoi
 * Utilisée par le serveur pour le signalement de la fin de jeu
 * @param bool result: le résultat de la partie pour le joueur
 * @return Message: la structure prête à l'envoi
 */
Message create_end_message(bool result);

/**
 * Fonction qui prépare la structure de message d'abandon
 * Utilisée par les clients lorsqu'il quitte inopinement
 * @return Message: la structure prête à l'envoi
 */
Message create_quit_message();

/**
 * Fonction qui prépare la structure de message texte
 * 
 * @param int length Taille du texte à envoyer (<= 100)
 * @param char text[100] Chaine à envoyer
 * @return Message: la structure prête à l'envoi
 */
Message create_text_message(int length, char text[100]);

/**
 * Fonction qui prépare la structure de message type
 * 
 * @param Type game_type Mode de jeu
 * @param int seconds Nombre de secondes par tour (0 si game_type == Type::Default)
 * @return Message: la structure prête à l'envoi
 */
Message create_type_message(Type game_type, int seconds);

/**
 *  Fonction utilisée dans la dé-sérialisation des données
 *  Prend un int simple et le converti en Vector2u
 *  @param gf::Vector2u *coo2D: le Vector2u à remplir
 *  @param int piece_pos: l'int de départ
 *  @param bool inversed
 *         Utilisé dans le serveur avec true pour le premier client
 *         et avec false pour le deuxième
 *         Si besoin dans le client, utiliser avec true
 */
void get_vector_coord(gf::Vector2u *coo2D, int piece_pos, bool inversed);

/**
 *  Fonction utilisée dans la sérialisation des données
 *  Prend un Vector2u et le transforme en un int
 *  @param gf::Vector2u *coo2D: le Vector2u de départ
 *  @param bool inversed
 *         Utilisé dans le serveur avec false pour le premier client
 *         et avec true pour le deuxième
 *         Si besoin dans le client, utiliser avec false
 *  @return int: l'int correspondant
 */
int get_pos_from_vector(gf::Vector2u *coo2D, bool inversed);

/**
 *  Fonction utisée dans la sérialisation des données
 *  Prend un Vector2u et une valeur pour résumer une pièce sous un int de position et un int de valeur
 *  @param gf::Vector2u *coo2D: le Vector2u de départ
 *  @param int value: la valeur de la pièce
 *  @param bool inversed
 *         Utilisé dans le serveur avec false pour le premier client
 *         et avec true pour le deuxième
 *         Si besoin dans le client, utiliser avec false
 *  @return Resumed_piece: la pièce résumée
 */
Resumed_piece create_resumed_piece(gf::Vector2u *coo2D, int value, bool inversed);

/**
 *  Fonction utilisée dans la sérialisation des données
 *  Prend 2 Vector2u pour crée une structure de mouvement avec deux int
 *  @param gf::Vector2u *source: La source du mouvement
 *  @param gf::Vector2u *target: La cible du mouvement
 *  @param bool inversed
 *         Utilisé dans le serveur avec false pour le premier client
 *         et avec true pour le deuxième
 *         Si besoin dans le client, utiliser avec false
 */
Movement create_movement(gf::Vector2u *source, gf::Vector2u *target, bool inversed);


#endif
