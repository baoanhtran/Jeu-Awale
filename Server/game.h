#ifndef GAME_H
#define GAME_H

#include "game_logic.h"
#include "client.h"

#define MAX_OBSERVATORS 100
#define MAX_DISCONNECTION 2 // Nombre max de déconnexions par joueur pendant une partie
#define MAX_ELAPSED_TIME 30 // Nombre de secondes d'attente après une déconnexion

typedef enum
{
    WIN,
    LOSE,
    DRAW
} Game_Result; // Pour connaître le résultat d'une partie

typedef enum
{
    PRIVATE,
    PUBLIC
} GameVisiblity; // Permet d'autoriser/bloquer l'observation aux clients inconnus

typedef struct
{
    int challenger_disconnections; // Nombre de déconnexions du challenger
    int challenged_disconnections;
    unsigned long elapsed_time; // Durée depuis la déconnexion
} GameDisconnection;

typedef enum
{
    IN_PROGRESS,
    PAUSED,
} GameState; // Indique si la partie est en cours ou en pause

typedef struct
{
    Client client_challenger;
    Client client_challenged;
    GameVisiblity visibility;
    Client **ptr_observators; // liste de pointeurs vers les observateurs (clients)
    int nb_observators;
    Partie *partie;
    GameDisconnection *game_disconnection;
    GameState game_state;
} Game;

void delete_game(Game *games, int to_remove, int *nb_games);
int has_game(Client client, const Game *games, int nb_games);
void play(const char *cell_str, Client client, Game *games, int nb_games, Client *clients, int actual);
void display_game(Game game); // envoie le plateau à tous les joueurs et observateurs
void display_turn(Game game);
void game_set_visiblity(Client sender, const char *visibility, Game *games, int nb_games); // définit la visibilité de la partie du sender
void show_all_games(Client sender, const Game *games, int nb_games);                       // affiche toutes les parties en cours
void observe_game(
    Client *clients, int actual, Game *games, int nb_games,
    int ind_sender, const char *name_player_in_game);                                               // permet au sender de devenir observateur de la partie de name_player_in_game
void remove_observator(Client *observator, Game *games, int nb_games);                              // retire un observateur d'une partie
void leave_observing(Client *ptr_sender, Game *games, int nb_games);                                // permet au sender de quitter le mode observation
void kick_all_observators(Game *ptr_game);                                                          // expulse tous les observateurs d'une partie
void is_observing_game(Client *ptr_client, Game *games, int nb_games, int *ind_game, int *ind_obs); // définit ind_game et ind_obs (ou -1 si non)
int is_in_game(const Game *games, int nb_games, Client client);
void init_game_disconnection(GameDisconnection *game_disconnection);
void refresh_paused_games(Client *clients, int actual, Game *games, int *nb_games);
static void end_paused_game(Client *clients, int actual, int ind_challenger, int ind_challenged, Game *games, int *nb_games, int game_idx);

#endif
