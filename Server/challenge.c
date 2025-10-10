#include "challenge.h"
#include "game.h"
#include "message.h"
#include "server.h"
#include <string.h>
#include <time.h>

#define MAX_PENDING_TIME 5

void challenged(const Client *clients, int actual, Client challenger,
                const char *challenged_name, Challenge *pending_challenges,
                int *nb_pending_challenges) {
  if (strcmp(challenger.name, challenged_name) != 0) {
    // Has the challenger challenged someone else ?
    if (challenger.status == IN_GAME) {
      // The challenger challenge someone while in game
      write_client(challenger.sock,
                   "You can't challenge someone else while in game.");
    } else {
      // Refresh the challenges to be sure the client's challenge it hasn't expired
      refresh_challenges(pending_challenges, nb_pending_challenges);
      // Returns the index of the challenge if the client has one, -1 otherwise
      int challenge_idx = has_pending_challenge(challenger, pending_challenges,
                                                nb_pending_challenges);
      if (challenge_idx == -1) {
        int client_idx = name_exists(clients, actual, challenged_name);
        // Has the challenger challenged a client that exists ?
        if (client_idx != -1) {
          Client challenged = clients[client_idx];
          int challenged_idx = has_pending_challenge(
              challenged, pending_challenges, nb_pending_challenges);
          // Does the challenger have a challenge ?
          if (challenged_idx == -1) {
            // Challenge sent
            send_challenge_message(challenger, challenged);
            if (challenged.status == ONLINE) {
              // Creates a challenge
              Challenge pending_challenge = {challenger, challenged, time(NULL)};
              pending_challenges[*nb_pending_challenges] = pending_challenge;
              (*nb_pending_challenges)++;
            }
          } else {
            // The challenged client already has a pending challenge
            write_client(challenger.sock,
                         "Another client has already sent a challenge to this "
                         "client. Wait few seconds...");
          }
        } else {
          // Name doesn't exist
          write_client(challenger.sock, "Client not found");
        }
      } else {
        Challenge challenge = pending_challenges[challenge_idx];
        if (strcmp(challenge.challenger.name, challenger.name) == 0) {
          // The challenger tried to challenge two clients in a row.
          write_client(challenger.sock, "You've already sent a challenge to a "
                                        "client. Wait few seconds...");
        } else {
          // The challenged client tried to challenge someone else while he has
          // a challenge.
          send_received_challenge(challenge);
        }
      }
    }
  } else {
    // Challenger challenged himself
    write_client(challenger.sock, "You can't challenge yourself you dumbass.");
  }
}

void accept_challenge(Client *clients, int actual, int client_idx, Challenge *pending_challenges,
                      int *nb_pending_challenges, Game games[], int *nb_games) {
  refresh_challenges(pending_challenges, nb_pending_challenges);
  char buffer[BUF_SIZE];
  Client client = clients[client_idx];
  int challenge_idx =
      has_pending_challenge(clients[client_idx], pending_challenges, nb_pending_challenges);
  if (challenge_idx != -1) {
    Challenge challenge = pending_challenges[challenge_idx];
    if (strcmp(clients[client_idx].name, challenge.challenger.name) == 0) {
      // The challenger tried to accept the challenge he has just sent.
      write_client(
          challenge.challenger.sock,
          "Either wait that your opponent accepts/denies the challenge or "
          "cancel it, but don't try to bypass the server...");
    } else {
      // The challenged client accepts the challenge.
      send_challenge_accepted_message(challenge);
      // Update the real status of challenger and challenged in clients
      int ind_challenger = name_exists(clients, actual, challenge.challenger.name);
      int ind_challenged = name_exists(clients, actual, challenge.challenged.name);
      clients[ind_challenger].status = IN_GAME;
      clients[ind_challenged].status = IN_GAME;

      // Runs the game...
      GameDisconnection *game_disconnection = (GameDisconnection *)malloc(sizeof(GameDisconnection));
      init_game_disconnection(game_disconnection);
      Game game = { clients[ind_challenger], clients[ind_challenged], PRIVATE, NULL, 0, creerPartie(), game_disconnection, IN_PROGRESS };
      cancel_challenge(client, pending_challenges, nb_pending_challenges);
      games[*nb_games] = game;
      (*nb_games)++;
      display_game(game);
      display_turn(game);
    }
  } else {
    // The client doesn't have any challenge
    write_client(
        clients[client_idx].sock,
        "You don't have any challenge request yet.");
  }
}


void deny_challenge(Client *clients, int client_idx, Challenge *pending_challenges, int *nb_pending_challenges){
  refresh_challenges(pending_challenges, nb_pending_challenges);
  char buffer[BUF_SIZE];
  Client client = clients[client_idx];
  int challenge_idx =
      has_pending_challenge(client, pending_challenges, nb_pending_challenges);
  if (challenge_idx != -1) {
    Challenge challenge = pending_challenges[challenge_idx];
    if (strcmp(client.name, challenge.challenger.name) == 0) {
      // The challenger tried to deny the challenge he has just sent.
      write_client(
          client.sock,
          "Either wait that your opponent accepts/denies the challenge or "
          "cancel it, but don't try to bypass the server...");
    } else {
      // The challenged client denies the challenge.
      send_challenge_denied_message(challenge);
      cancel_challenge(client, pending_challenges, nb_pending_challenges);
    }
  } else {
    write_client(
        client.sock,
        "You don't have any challenge request yet.");
  }
}

// Returns the index of the challenge if the client is in one of the pending challenges, -1 otherwise
int has_pending_challenge(Client client, Challenge *pending_challenges,
                          int *nb_pending_challenges) {
  for (int i = 0; i < *nb_pending_challenges; i++) {
    if (strcmp(pending_challenges[i].challenger.name, client.name) == 0 ||
        strcmp(pending_challenges[i].challenged.name, client.name) == 0) {
      return i;
    }
  }
  return -1;
}

// Removes a challenge from the array
void remove_challenge(Challenge *pending_challenges, int to_remove,
                      int *nb_pending_challenges) {
  memmove(pending_challenges + to_remove, pending_challenges + to_remove + 1,
          (*nb_pending_challenges - to_remove - 1) * sizeof(Challenge));
  (*nb_pending_challenges)--;
}

// Cancel a challenge
void cancel_challenge(Client client, Challenge *pending_challenges,
                      int *nb_pending_challenges) {
  int i =
      has_pending_challenge(client, pending_challenges, nb_pending_challenges);
  if (i != -1) {
    remove_challenge(pending_challenges, i, nb_pending_challenges);
    i--;
  }
}

// Refresh the challenges
void refresh_challenges(Challenge *pending_challenges,
                        int *nb_pending_challenges) {
  for (int i = 0; i < *nb_pending_challenges; i++) {
    unsigned long elapsed_time = difftime(time(NULL), pending_challenges[i].timestamp);
      // If at least one client is offline or the timestamp exceed the expiration time
    if (elapsed_time > MAX_PENDING_TIME || pending_challenges[i].challenger.status == OFFLINE ||
        pending_challenges[i].challenged.status == OFFLINE){
      if(elapsed_time > MAX_PENDING_TIME){
        // Notify both client that the challenge expired
        write_client(pending_challenges[i].challenger.sock, "Challenge request expired.\n");
        write_client(pending_challenges[i].challenged.sock, "Challenge request expired.\n");
      }
      // The remove it
      remove_challenge(pending_challenges, i, nb_pending_challenges);
      i--;
    }
  }
}
