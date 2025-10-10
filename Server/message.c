#include <string.h>

#include "message.h"
#include "server.h"
#include "user_interaction.h"


char buffer[BUF_SIZE];

void send_menu_commands(SOCKET sock) {
  //show all commands of game to a client                                                                
  write_client(
    sock,
    "List of commands:\n"
    "  - menu: see all commands.\n"
    "  - names: see all connected clients and their status.\n"
    "  - rank: display top 50 players.\n"
    "  - challenge [pseudo]: challenge a client for a game. Play with commands:\n"
    "       + play [cell_number]: Move seeds in cell_number.\n"
    "  - games: see all games in progress.\n"
    "  - set visibility [private/public]: set visibility of playing game.\n"
    "  - observe [pseudo]: participate the game of a user as an observator.\n"
    "  - leave: leave observing mode.\n"
    "  - chat [pseudo] [message]: send message to a client.\n"
    "  - set bio [description]: set description of your bio.\n"
    "  - bio [pseudo]: see bio of a client.\n"
    "  - clear: clear terminal.\n"
    "  - make friend [pseudo]: send a friend request to a client.\n"
    "       + accept fr [pseudo]: accept friend request of someone.\n"
    "       + deny fr [pseudo]: deny friend request of someone.\n"
    "       + cancel fr [pseudo]: cancel your friend request to someone.\n"
    "  - unfriend [pseudo]: unfriend with pseudo.\n"
    "  - friends: show list of my friends.\n"
    "  - friend requests: show list of my friend requests.\n"
  );
}

void send_challenge_message(Client challenger, Client challenged){
 switch(challenged.status){
  case OFFLINE:
    write_client(challenger.sock, "This client is offline.");
    break;
  case IN_GAME:
    write_client(challenger.sock, "This client is already in game.");
    break;
  case OBSERVING:
    write_client(challenger.sock, "This client is watching a game.");
    break;
  case ONLINE:
    strncpy(buffer, challenger.name, BUF_SIZE - 1);
    strncat(buffer, " has challenged you. [accept/deny]",
            BUF_SIZE - strlen(buffer) - 1);
    write_client(challenged.sock, buffer);
    write_client(challenger.sock, "Challenge request sent.");
    break;
  }
}

void send_received_challenge(Challenge challenge){
  strncpy(buffer, challenge.challenger.name, BUF_SIZE - 1);
  strncat(buffer, " has already sent you a challenge request. [accept/deny] it before challenging someone else.", BUF_SIZE - strlen(buffer) - 1);
  write_client(challenge.challenged.sock, buffer);
}

void send_challenge_accepted_message(Challenge challenge){
  strncpy(buffer, challenge.challenged.name, BUF_SIZE - 1);
  strncat(buffer, " has accepted the challenge. Joining the game...",
          BUF_SIZE - strlen(buffer) - 1);
  write_client(challenge.challenger.sock, buffer);
  strncpy(buffer, "Challenge accepted. Joining the game...", BUF_SIZE - 1);
  write_client(challenge.challenged.sock, buffer);
}

void send_challenge_denied_message(Challenge challenge){
  strncpy(buffer, challenge.challenged.name, BUF_SIZE - 1);
  strncat(buffer, " has denied the challenge.",
          BUF_SIZE - strlen(buffer) - 1);
  write_client(challenge.challenger.sock, buffer);
  write_client(challenge.challenged.sock, "Challenge denied.");
}

void send_to_all_observators(Game game, const char *buffer) {
	//Send msg in buffer to all observators in game

	for (int i = 0; i < game.nb_observators; i++) {
		Client obs = *(game.ptr_observators[i]);
		if (obs.status == OBSERVING) {
			write_client(obs.sock, buffer);
		}
	}
}

void send_msg_friend_request( Client sender, Client requested ) { 
  //Send content of friend request to sender and client
	fwrite_client( sender.sock, "Friend request send!" );
  if (requested.status != OFFLINE) {
    fwrite_client( requested.sock, "%s sent you a friend request! [accept fr %s / deny fr %s ]? You can ignore this and accept later.", sender.name, sender.name, sender.name );
  }
}

void send_list_friend_request( Client sender ) {
  //send list of friend request to sender

  //check if sender has any friend request
  if (sender.nb_friend_req == 0) {
    write_client(sender.sock, "You dont have any friend request yet!");
    return;
  }

  //count friend req not deleted
  int count = 0;
  for (int i = 0; i < sender.nb_friend_req; i++) {
    if (sender.friend_req[i].status != DELETED) {
      count++;
    }
  }

  if (count == 0) {
    write_client(sender.sock, "You dont have any friend request yet!");
    return;
  }

  //send list of friend requests
  char buffer[BUF_SIZE];
  sprintf(buffer, "You have %d friend requests: \n", count);
  for (int i = 0; i < sender.nb_friend_req; i++) {
    Friend_Req fr = sender.friend_req[i];

    //convert enum to string
    char status[10];
    switch (fr.status) {
      case ACCEPTED:
        strcpy(status, "ACCEPTED");
        break;
      case DENIED:
        strcpy(status, "DENIED");
        break;
      case PENDING:
        strcpy(status, "PENDING");
        break;
      default:
        strcpy(status, "DELETED");
        break;
    }
    char info[BUF_SIZE];
    if (fr.status != DELETED) { //do not send deleted friend requests 
      sprintf(info, 
        "  - %s [%s]\n",
        fr.name_client, status
      );
      strncat(buffer, info, BUF_SIZE-1);
    }
  }
  write_client(sender.sock, buffer);
}

void send_list_friend( Client sender, Client clients[], int actual) {
  //send list of friend of sender to sender

  //check if sender has friend or not
  if (sender.nb_friend_req == 0) {
    write_client(sender.sock, "You dont have any friend yet. Let make friend!");
    return;
  }

  //count friends
  int count = 0;
  for (int i = 0; i < sender.nb_friend_req; i++) {
    if (sender.friend_req[i].status == ACCEPTED) {
      count++;
    }
  }

  if (count == 0) {
    write_client(sender.sock, "You dont have any friend yet. Let make friend!");
    return;
  }

  //send list of friends
  char buffer[BUF_SIZE];
  sprintf(buffer, "You have %d fiends: \n", count);

  for (int i = 0; i < sender.nb_friend_req; i++) {
    Friend_Req fr = sender.friend_req[i];
    int ind_client = name_exists(clients, actual, fr.name_client);

    //convert emnum to string
    char info[BUF_SIZE];
    if (fr.status == ACCEPTED) {
      char status[10];
      switch (clients[ind_client].status) {
        case ONLINE:
          strcpy(status, "ONLINE");
          break;
        case OFFLINE:
          strcpy(status, "OFFLINE");
          break;
        case OBSERVING:
          strcpy(status, "OBSERVING");
          break;
        case IN_GAME:
          strcpy(status, "IN_GAME");
          break;
      }

      sprintf(info, "  - [%s] %s\n", status, fr.name_client);
      strncat(buffer, info, BUF_SIZE-1);
    }
  }
  write_client(sender.sock, buffer);
}

