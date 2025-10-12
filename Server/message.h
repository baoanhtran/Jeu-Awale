#ifndef MESSAGE_H
#define MESSAGE_H

#include "challenge.h"
#include "server.h"

// messages de base
void send_menu_commands(SOCKET sock); // affiche toutes les commandes disponibles au client

// messages de défi
void send_challenge_message(Client challenger, Client challenged);
void send_received_challenge(Challenge challenge);
void send_challenge_accepted_message(Challenge challenge);
void send_challenge_denied_message(Challenge challenge);

// messages de jeu
void send_to_all_observators(Game game, const char *buffer); // envoie buffer à tous les observateurs d'une partie

// messages d'amis
void send_msg_friend_request(Client sender, Client requested);      // envoie le contenu d'une demande d'ami au sender et au client
void send_list_friend_request(Client sender);                       // envoie la liste des demandes d'ami au sender
void send_list_friend(Client sender, Client clients[], int actual); // envoie la liste d'amis du sender

#endif // !MESSAGE
