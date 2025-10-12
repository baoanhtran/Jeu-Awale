#include <limits.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "utils.h"
#include "challenge.h"
#include "game.h"
#include "message.h"
#include "user_interaction.h"
#include "ranking.h"
#include "server.h"

// Delete a game
void delete_game(Game *games, int to_remove, int *nb_games)
{
    free(games[to_remove].partie);
    free(games[to_remove].ptr_observators);
    memmove(games + to_remove, games + to_remove + 1,
            (*nb_games - to_remove - 1) * sizeof(Game));
    (*nb_games)--;
}

void play(const char *cell_str, Client client, Game *games, int nb_games, Client *clients, int actual)
{
    char buffer[BUF_SIZE];
    int game_idx = has_game(client, games, nb_games);
    if (game_idx != -1)
    {
        // Le client est en partie.
        Game game = games[game_idx];
        Partie *partie = game.partie;
        bool client_is_challenger = (strcmp(game.client_challenger.name, client.name) == 0);
        bool client_is_challenged = (strcmp(game.client_challenged.name, client.name) == 0);
        bool challenger_turn = partie->tour_joueur1;
        // Si la partie n'est pas en pause, les deux clients sont connectés et peuvent jouer
        if (game.game_state != PAUSED)
        {
            // Vérifie que c'est bien le tour du client qui essaie de jouer
            if ((client_is_challenged && !challenger_turn) || (client_is_challenger && challenger_turn))
            {
                int cell;
                my_atoi(cell_str, &cell); // conversion string -> int
                // Le coup est valide
                if (verifierCoup(partie, cell))
                {
                    effectuerTour(partie, cell);
                    display_game(game);
                    // La partie est-elle terminée ?
                    if (finDePartie(partie))
                    {
                        Game_Result result_of_challenger;
                        // Match nul
                        if (partie->score_joueur1 == partie->score_joueur2)
                        {
                            write_client(game.client_challenger.sock, "Match nul.");
                            write_client(game.client_challenged.sock, "Match nul.");
                            result_of_challenger = DRAW;
                        }
                        else
                        {
                            // Challenger gagne
                            if (partie->score_joueur1 > partie->score_joueur2)
                            {
                                write_client(game.client_challenger.sock, "Vous avez gagné");
                                write_client(game.client_challenged.sock, "Vous avez perdu");
                                sprintf(buffer, "%s a gagné", game.client_challenger.name);
                                send_to_all_observators(game, buffer);
                                result_of_challenger = WIN;
                            }
                            else
                            {
                                // Challenged gagne
                                write_client(game.client_challenger.sock, "Vous avez perdu");
                                write_client(game.client_challenged.sock, "Vous avez gagné");
                                sprintf(buffer, "%s a gagné", game.client_challenged.name);
                                send_to_all_observators(game, buffer);
                                result_of_challenger = LOSE;
                            }
                        }
                        // Remet les deux clients en ligne, met à jour les ELO, envoie les messages de fin...
                        int ind_challenger = name_exists(clients, actual, game.client_challenger.name);
                        int ind_challenged = name_exists(clients, actual, game.client_challenged.name);
                        clients[ind_challenger].status = ONLINE;
                        clients[ind_challenged].status = ONLINE;
                        updateElo(&clients[ind_challenger], &clients[ind_challenged], result_of_challenger, K_COEF);
                        free(game.game_disconnection);
                        kick_all_observators(&games[game_idx]);

                        // Message de fin de partie aux observateurs
                        send_to_all_observators(games[game_idx], "Partie terminée. Vous quittez le mode observation !\n");
                        write_client(client.sock, "Fin de la partie. Vous pouvez maintenant défier d'autres joueurs.\n");
                        delete_game(games, game_idx, &nb_games);
                    }
                    else
                    {
                        // On notifie le prochain joueur que c'est à lui de jouer
                        display_turn(game);
                    }
                }
                else
                {
                    // Le coup n'est pas valide
                    strncpy(buffer, "Cette case est invalide. Réessayez.", BUF_SIZE - 1);
                    write_client(client.sock, buffer);
                }
            }
            else
            {
                // Le client essaie de jouer pendant le tour de son adversaire.
                strncpy(buffer, "C'est au tour de votre adversaire. Attendez quelques instants...", BUF_SIZE - 1);
                write_client(client.sock, buffer);
            }
        }
        else
        {
            // Le client tente de jouer alors que la partie est en pause.
            write_client(client.sock, "Vous ne pouvez pas jouer pendant que la partie est en pause.");
        }
    }
    else
    {
        // Le client n'est pas en partie.
        strncpy(buffer, "Vous n'êtes pas actuellement en partie.", BUF_SIZE - 1);
        write_client(client.sock, buffer);
    }
}

// Returns the index of the game if the client is in game, -1 otherwise
int has_game(Client client, const Game *games, int nb_games)
{
    for (int i = 0; i < nb_games; i++)
    {
        if (strcmp(games[i].client_challenger.name, client.name) == 0 || strcmp(games[i].client_challenged.name, client.name) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Ecrit le plateau de la partie aux deux joueurs et aux observateurs
void display_game(Game game)
{

    // player 1
    char buf_1[BUF_SIZE];
    getPlateau(buf_1, game.partie, 1, game.client_challenger.name, game.client_challenged.name); // On récupère le buffer correspondant au plateau
    write_client(game.client_challenger.sock, buf_1);

    // player 2
    char buf_2[BUF_SIZE];
    getPlateau(buf_2, game.partie, 2, game.client_challenger.name, game.client_challenged.name); // On récupère le buffer correspondant au plateau
    write_client(game.client_challenged.sock, buf_2);

    // observators
    char buf_obs[BUF_SIZE];
    getPlateau(buf_obs, game.partie, 0, game.client_challenger.name, game.client_challenged.name); // On récupère le buffer correspondant au plateau
    send_to_all_observators(game, buf_obs);
}

// Indique à qui est le tour de jouer
void display_turn(Game game)
{
    char buffer[BUF_SIZE];
    if (game.partie->tour_joueur1)
    {
        fwrite_client(game.client_challenger.sock, "C'est votre tour. Pour jouer, utilisez la commande 'play [6-11]'");
        sprintf(buffer, "C'est le tour de %s.", game.client_challenger.name);
        fwrite_client(game.client_challenged.sock, buffer);
        send_to_all_observators(game, buffer);
    }
    else
    {
        fwrite_client(game.client_challenged.sock, "C'est votre tour. Pour jouer, utilisez la commande 'play [0-5]'");
        sprintf(buffer, "C'est le tour de %s.", game.client_challenged.name);
        fwrite_client(game.client_challenger.sock, buffer);
        send_to_all_observators(game, buffer);
    }
}

// Définit la visibilité de la partie
void game_set_visiblity(Client sender, const char *visibility, Game *games, int nb_games)
{
    // définir la visibilité de la partie du sender

    // vérifie les paramètres
    if (*visibility == '\0')
    {
        fwrite_client(sender.sock, "Visibilité introuvable ! Syntaxe : set visibility [private/public].");
        return;
    }

    // vérifie si la visibilité est valide
    GameVisiblity gv = PUBLIC;
    if (strcmp(visibility, "private") == 0)
    {
        gv = PRIVATE;
    }
    else if (strcmp(visibility, "public") == 0)
    {
        gv = PUBLIC;
    }
    else
    {
        fwrite_client(sender.sock, "Visibilité '%s' inconnue ! Essayez 'private' ou 'public'.", visibility);
        return;
    }

    // vérifie si le sender est en partie
    int ind_game = has_game(sender, games, nb_games);
    if (ind_game == -1)
    {
        fwrite_client(sender.sock, "On dirait que vous n'êtes pas en partie. La visibilité concerne uniquement la partie en cours.");
        return;
    }

    // convertit enum en chaîne
    char info[BUF_SIZE];
    switch (gv)
    {
    case PRIVATE:
        strcpy(info, "privée");
        break;
    case PUBLIC:
        strcpy(info, "publique");
        break;
    default:
        break;
    }

    if (games[ind_game].visibility == gv)
    {
        fwrite_client(sender.sock, "La visibilité de la partie est déjà %s.", info);
    }
    else
    {
        games[ind_game].visibility = gv;
        fwrite_client(sender.sock, "La visibilité de la partie est définie sur %s.", info);

        // expulse les observateurs non amis si on passe en privé
        if (gv == PRIVATE)
        {
            for (int i = 0; i < games[ind_game].nb_observators; i++)
            {
                int is_friend_j1 = is_friend(*(games[ind_game].ptr_observators[i]), games[ind_game].client_challenged.name);
                int is_friend_j2 = is_friend(*(games[ind_game].ptr_observators[i]), games[ind_game].client_challenger.name);
                if (is_friend_j1 == -1 && is_friend_j2 == -1)
                {
                    char msg[BUF_SIZE];
                    snprintf(msg, BUF_SIZE, "La visibilité de la partie est passée en privée. Vous n'êtes ami ni avec %s ni avec %s, vous quittez le mode observation !",
                             games[ind_game].client_challenged.name, games[ind_game].client_challenger.name);
                    write_client(games[ind_game].ptr_observators[i]->sock, msg);
                    games[ind_game].ptr_observators[i]->status = ONLINE;
                    // retire l'observateur de la liste
                    games[ind_game].ptr_observators[i] = games[ind_game].ptr_observators[games[ind_game].nb_observators - 1];
                    games[ind_game].nb_observators--;
                    i--;
                }
            }
        }
    }
}

// Displays current games
void show_all_games(Client sender, const Game *games, int nb_games)
{
    // show all games are in process to the sender

    char buffer[BUF_SIZE];

    sprintf(buffer, nb_games > 0
                        ? "Toutes les parties en cours :\n"
                        : "Aucune partie en cours pour le moment !");
    for (int i = 0; i < nb_games; i++)
    {
        char info[NAME_SIZE * 3];
        sprintf(info, "Partie %d : %s vs. %s (%s)\n", i, games[i].client_challenged.name, games[i].client_challenger.name, games[i].visibility == PRIVATE ? "privée" : "publique");
        strncpy(buffer, info, BUF_SIZE - 1);
    }
    write_client(sender.sock, buffer);
}

void observe_game(
    Client *clients, int actual, Game *games, int nb_games,
    int ind_sender, const char *name_player_in_game)
{
    // let sender becomes the observator of game in which player_in_game is playing
    switch (clients[ind_sender].status)
    {
    case IN_GAME:
        write_client(clients[ind_sender].sock, "Vous êtes dans une partie, vous ne pouvez pas regarder une autre partie !");
        return;
    case OBSERVING:
        write_client(clients[ind_sender].sock, "Vous regardez déjà une partie, veuillez quitter pour en regarder une autre !");
        return;
    default:
        break;
    }

    // check if player exist!
    int player_id = name_exists(clients, actual, name_player_in_game);
    if (player_id == -1)
    {
        fwrite_client(clients[ind_sender].sock, "Le joueur %s n'existe pas !", name_player_in_game);
        return;
    }
    Client player_in_game = clients[player_id];
    int game_id = has_game(player_in_game, games, nb_games);

    if (game_id == -1)
    {
        fwrite_client(clients[ind_sender].sock,
                      "On dirait que %s n'est dans aucune partie !",
                      player_in_game.name);
        return;
    }
    Game g = games[game_id];

    // check if friend for game mode PRIVATE
    if (g.visibility == PRIVATE)
    {
        int is_friend_j1 = is_friend(clients[ind_sender], g.client_challenged.name);
        int is_friend_j2 = is_friend(clients[ind_sender], g.client_challenger.name);
        if (is_friend_j1 == -1 && is_friend_j2 == -1)
        {
            fwrite_client(clients[ind_sender].sock,
                          "Vous ne pouvez pas observer la partie car vous n'êtes ami ni avec %s ni avec %s !",
                          g.client_challenged.name, g.client_challenger.name);
            return;
        }
    }

    // if sender is already in the list of observators before (he want to return back after leaving)
    for (int i = 0; i < g.nb_observators; i++)
    {
        if (g.ptr_observators[i]->name == clients[ind_sender].name)
        {
            clients[ind_sender].status = OBSERVING;
            return;
        }
    }

    // if reached max number of observators
    if (g.nb_observators == MAX_OBSERVATORS)
    {
        fwrite_client(clients[ind_sender].sock,
                      "Le nombre d'observateurs a atteint la limite !");
        return;
    }

    // create list of observators if not create
    if (g.ptr_observators == NULL)
    {
        games[game_id].ptr_observators = malloc(MAX_OBSERVATORS * sizeof(Client *));
    }

    clients[ind_sender].status = OBSERVING;
    games[game_id].ptr_observators[g.nb_observators] = &clients[ind_sender];
    games[game_id].nb_observators++;

    char buffer[BUF_SIZE];
    getPlateau(buffer, g.partie, 0, g.client_challenger.name, g.client_challenged.name); // On récupère le buffer correspondant au plateau
    write_client(clients[ind_sender].sock, buffer);
}

void remove_observator(Client *ptr_observator, Game *games, int nb_games)
{
    // remove observator from game
    int ind_game, ind_obs;
    is_observing_game(ptr_observator, games, nb_games, &ind_game, &ind_obs);
    games[ind_game].ptr_observators[ind_obs] = games[ind_game].ptr_observators[games[ind_game].nb_observators - 1];
    games[ind_game].nb_observators--;
    ptr_observator->status = ONLINE;
}

void leave_observing(Client *ptr_sender, Game *games, int nb_games)
{
    // let sender escape observating mode
    switch (ptr_sender->status)
    {
    case IN_GAME:
        write_client(ptr_sender->sock, "Vous êtes dans une partie, ceci est uniquement pour quitter une observation !");
        return;
    case OBSERVING:
        remove_observator(ptr_sender, games, nb_games);
        write_client(ptr_sender->sock, "Vous avez quitté l'observation !");
        return;
    default:
        write_client(ptr_sender->sock, "Vous n'observez aucune partie !");
        break;
    }
    ptr_sender->status = ONLINE;
}

void kick_all_observators(Game *ptr_game)
{
    // kick all observers of game

    for (int i = 0; i < ptr_game->nb_observators; i++)
    {
        write_client(ptr_game->ptr_observators[i]->sock, "La partie est terminée. Vous quittez maintenant le mode d'observation !");
        ptr_game->ptr_observators[i]->status = ONLINE;
    }
    free(ptr_game->ptr_observators);
    ptr_game->ptr_observators = NULL;
    ptr_game->nb_observators = 0;
}

int is_in_game(const Game *games, int nb_games, Client client)
{
    // Returns the index of a game if the client is in game, -1 otherwise
    for (int i = 0; i < nb_games; i++)
    {
        if (strcmp(games[i].client_challenged.name, client.name) == 0 || strcmp(games[i].client_challenger.name, client.name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void is_observing_game(Client *ptr_client, Game *games, int nb_games, int *ind_game, int *ind_obs)
{
    // Returns the index of a game and the index of the client in the game if the client is observing, -1 otherwise

    for (int i = 0; i < nb_games; i++)
    {
        for (int j = 0; j < games[i].nb_observators; j++)
        {
            if (ptr_client == games[i].ptr_observators[j])
            {
                *ind_game = i;
                *ind_obs = j;
                return;
            }
        }
    }
    *ind_game = -1;
    *ind_obs = -1;
}

// Initialize GameDisconnection (to handle in game disconnections )
void init_game_disconnection(GameDisconnection *game_disconnection)
{
    game_disconnection->challenger_disconnections = 0;
    game_disconnection->challenged_disconnections = 0;
}

// Refresh paused games
void refresh_paused_games(Client *clients, int actual, Game *games, int *nb_games)
{
    for (int i = 0; i < *nb_games; i++)
    {
        // If the game is paused (= a client has disconnected) and the waiting time succeed, we end the game
        if (games[i].game_state == PAUSED && time(NULL) - games[i].game_disconnection->elapsed_time > MAX_ELAPSED_TIME)
        {
            int ind_challenger = name_exists(clients, actual, games[i].client_challenger.name);
            int ind_challenged = name_exists(clients, actual, games[i].client_challenged.name);
            end_paused_game(clients, actual, ind_challenger, ind_challenged, games, nb_games, i);
            i--;
        }
    }
}

// Ends a game prematurly due to : (1) too many disconnections or (2) a long disconnection time
static void end_paused_game(Client *clients, int actual, int ind_challenger, int ind_challenged, Game *games, int *nb_games, int game_idx)
{
    if (games[game_idx].client_challenged.status == OFFLINE)
    {
        write_client(games[game_idx].client_challenger.sock, "Votre adversaire met trop de temps à se reconnecter. Vous gagnez.\n");
        clients[ind_challenger].status = ONLINE;
    }
    else
    {
        write_client(games[game_idx].client_challenged.sock, "Votre adversaire met trop de temps à se reconnecter. Vous gagnez.\n");
        clients[ind_challenged].status = ONLINE;
    }
    send_to_all_observators(games[game_idx], "Le joueur a mis trop de temps à se reconnecter.\n");
    kick_all_observators(&games[game_idx]);
    updateElo(&clients[ind_challenger], &clients[ind_challenged], WIN, K_COEF);
    delete_game(games, game_idx, nb_games);
}
