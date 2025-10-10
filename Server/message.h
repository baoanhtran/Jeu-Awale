#ifndef MESSAGE_H
#define MESSAGE_H

#include "challenge.h"
#include "server.h"

// base messages
void send_menu_commands(SOCKET sock); // show all commands of game to a client

// challenge messages
void send_challenge_message(Client challenger, Client challenged);
void send_received_challenge(Challenge challenge);
void send_challenge_accepted_message(Challenge challenge);
void send_challenge_denied_message(Challenge challenge);

// game messages
void send_to_all_observators(Game game, const char *buffer); // Send msg in buffer to all observators in game

// friends messages
void send_msg_friend_request(Client sender, Client requested);      // Send content of friend request to sender and client
void send_list_friend_request(Client sender);                       // send list of friend request to sender
void send_list_friend(Client sender, Client clients[], int actual); // send list of friend of sender to sender

#endif // !MESSAGE
