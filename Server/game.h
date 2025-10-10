#ifndef GAME_H
#define GAME_H

#include "../awale.h"
#include "client.h"

#define MAX_OBSERVATORS    100
#define MAX_DISCONNECTION 2  // Maximum number of disconnections for each player during a game
#define MAX_ELAPSED_TIME 30  // Numbers of seconds to wait after a disconnection

typedef enum {
    WIN,
    LOSE,
    DRAW
} Game_Result; // To know the result of a game

typedef enum {
	PRIVATE,
    PUBLIC
} GameVisiblity; // To allow/block observations of unknown clients

typedef struct {
  int challenger_disconnections;  // How many times the challenger has disconnected ?
  int challenged_disconnections;
  unsigned long elapsed_time; // How long the disconnected client has been disconnected ?
} GameDisconnection;

typedef enum {
  IN_PROGRESS,
  PAUSED,
} GameState; // To know if a client is disconnected or not

typedef struct {
  Client client_challenger;
  Client client_challenged;
  GameVisiblity visibility;
  Client **ptr_observators; //liste de pointers des observators (clients)
  int nb_observators;
  Partie *partie;
  GameDisconnection *game_disconnection;
  GameState game_state;
} Game;

Game create_game(Client client_challenger, Client client_challenged, Game games[], int *games_nb); // TODO
void delete_game(Game *games, int to_remove, int *nb_games);
int has_game(Client client, const Game *games, int nb_games);
void play(const char *cell_str, Client client, Game *games, int nb_games, Client *clients, int actual);
void display_game(Game game); //send plateau to all players and observators
void display_turn(Game game);
void game_set_visiblity(Client sender, const char * visibility, Game *games, int nb_games); //set visiblity of game playing par sender
void show_all_games(Client sender, const Game *games, int nb_games); //show all games are in process to the sender
void observe_game(
	Client* clients, int actual, Game *games, int nb_games, 
	int ind_sender, const char *name_player_in_game
); //let sender becomes the observator of game in which player_in_game is playing
void remove_observator(Client* observator, Game *games, int nb_games); //remove observator from game
void leave_observing(Client* ptr_sender, Game *games, int nb_games); //let sender escape observating mode
void kick_all_observators(Game *ptr_game); //kick all observators of game
void is_observing_game(Client* ptr_client, Game *games, int nb_games, int* ind_game, int* ind_obs); //set ind_game and ind_obs to the index of game and observator in game (-1 if not)
int is_in_game(const Game *games, int nb_games, Client client); 
void init_game_disconnection(GameDisconnection *game_disconnection); 
void refresh_paused_games(Client *clients, int actual, Game *games, int *nb_games);
static void end_paused_game(Client *clients, int actual, int ind_challenger, int ind_challenged, Game *games, int *nb_games, int game_idx);

#endif
