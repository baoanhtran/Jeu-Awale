#ifndef SERVER_CHALLENGE_H
#define SERVER_CHALLENGE_H

#include "game.h"
typedef struct {
  Client challenger; // Copy of the client who initated the challenge
  Client challenged; // Copy of the client who has been challenged
  time_t timestamp; // Timestamp of the challenge (used for challenge expiration)
} Challenge;

// Manage the whole challenging process
void challenged(const Client *clients, int actual, Client challenger, const char *challenged_name, Challenge *pending_challenges, int *nb_pending_challenges); 
// Manage when the challenged client accepts the challenge
void accept_challenge(Client *clients, int actual, int client_idx, Challenge *pending_challenges, int *nb_pending_challenges, Game games[], int *nb_games);
// Manage when the challenged client denies the challenge
void deny_challenge(Client *clients, int client_idx, Challenge *pending_challenges, int *nb_pending_challenges);
// Removes the challenge from the Challenge array
void remove_challenge(Challenge *pending_challenges, int to_remove, int *nb_pending_challenges);
// Refresh the challenges (challenge expiration, disconnections while challenging/challenged)
void refresh_challenges(Challenge *pending_challenges, int *nb_pending_challenges);
// Check if the challenge has expired
bool has_challenge_expired(Challenge *pending_challenges, int to_check, int *nb_pending_challenges);
// Check if a given client has a pending challenge
int has_pending_challenge(Client client, Challenge *pending_challenges, int *nb_pending_challenges);
// Cancels a challenge
void cancel_challenge(Client client, Challenge *pending_challenges, int *nb_pending_challenges);

#endif 

