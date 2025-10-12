#include "challenge.h"
#include "game.h"
#include "message.h"
#include "server.h"
#include <string.h>
#include <time.h>

#define MAX_PENDING_TIME 5

void challenged(const Client *clients, int actual, Client challenger,
                const char *challenged_name, Challenge *pending_challenges,
                int *nb_pending_challenges)
{
    if (strcmp(challenger.name, challenged_name) != 0)
    {
        // Le challenger a-t-il défié quelqu'un d'autre ?
        if (challenger.status == IN_GAME)
        {
            // Le challenger défie quelqu'un alors qu'il est en jeu
            write_client(challenger.sock,
                         "Vous ne pouvez pas défier quelqu'un d'autre pendant une partie.");
        }
        else
        {
            // Rafraîchir les défis pour s'assurer que le défi du client n'a pas expiré
            refresh_challenges(pending_challenges, nb_pending_challenges);
            // Retourne l'index du défi si le client en a un, -1 sinon
            int challenge_idx = has_pending_challenge(challenger, pending_challenges,
                                                      nb_pending_challenges);
            if (challenge_idx == -1)
            {
                int client_idx = name_exists(clients, actual, challenged_name);
                // Le challenger a-t-il défié un client qui existe ?
                if (client_idx != -1)
                {
                    Client challenged = clients[client_idx];
                    int challenged_idx = has_pending_challenge(
                        challenged, pending_challenges, nb_pending_challenges);
                    // Est-ce que le client défié a déjà un défi ?
                    if (challenged_idx == -1)
                    {
                        // Défi envoyé
                        send_challenge_message(challenger, challenged);
                        if (challenged.status == ONLINE)
                        {
                            // Crée un défi
                            Challenge pending_challenge = {challenger, challenged, time(NULL)};
                            pending_challenges[*nb_pending_challenges] = pending_challenge;
                            (*nb_pending_challenges)++;
                        }
                    }
                    else
                    {
                        // Le client défié a déjà un défi en attente
                        write_client(challenger.sock,
                                     "Un autre client a déjà envoyé un défi à ce client. Attendez quelques secondes...");
                    }
                }
                else
                {
                    // Nom introuvable
                    write_client(challenger.sock, "Client introuvable");
                }
            }
            else
            {
                Challenge challenge = pending_challenges[challenge_idx];
                if (strcmp(challenge.challenger.name, challenger.name) == 0)
                {
                    // Le challenger a essayé de défier deux clients à la suite.
                    write_client(challenger.sock, "Vous avez déjà envoyé un défi à un client. Attendez quelques secondes...");
                }
                else
                {
                    // Le client défié a essayé de défier quelqu'un d'autre alors qu'il a un défi.
                    send_received_challenge(challenge);
                }
            }
        }
    }
    else
    {
        // Le challenger s'est défié lui-même
        write_client(challenger.sock, "Vous ne pouvez pas vous défier vous-même.");
    }
}

void accept_challenge(Client *clients, int actual, int client_idx, Challenge *pending_challenges,
                      int *nb_pending_challenges, Game games[], int *nb_games)
{
    refresh_challenges(pending_challenges, nb_pending_challenges);
    char buffer[BUF_SIZE];
    Client client = clients[client_idx];
    int challenge_idx =
        has_pending_challenge(clients[client_idx], pending_challenges, nb_pending_challenges);
    if (challenge_idx != -1)
    {
        Challenge challenge = pending_challenges[challenge_idx];
        if (strcmp(clients[client_idx].name, challenge.challenger.name) == 0)
        {
            // Le challenger a essayé d'accepter le défi qu'il vient d'envoyer.
            write_client(
                challenge.challenger.sock,
                "Attendez que votre adversaire accepte/refuse le défi ou annulez-le, mais n'essayez pas de contourner le serveur...");
        }
        else
        {
            // Le client défié accepte le défi.
            send_challenge_accepted_message(challenge);
            // Met à jour le statut réel du challenger et du défié dans clients
            int ind_challenger = name_exists(clients, actual, challenge.challenger.name);
            int ind_challenged = name_exists(clients, actual, challenge.challenged.name);
            clients[ind_challenger].status = IN_GAME;
            clients[ind_challenged].status = IN_GAME;

            // Lancer la partie...
            GameDisconnection *game_disconnection = (GameDisconnection *)malloc(sizeof(GameDisconnection));
            init_game_disconnection(game_disconnection);
            Game game = {clients[ind_challenger], clients[ind_challenged], PRIVATE, NULL, 0, creerPartie(), game_disconnection, IN_PROGRESS};
            cancel_challenge(client, pending_challenges, nb_pending_challenges);
            games[*nb_games] = game;
            (*nb_games)++;
            display_game(game);
            display_turn(game);
        }
    }
    else
    {
        // Le client n'a aucune demande de défi pour le moment
        write_client(
            clients[client_idx].sock,
            "Vous n'avez aucune demande de défi pour le moment.");
    }
}

void deny_challenge(Client *clients, int client_idx, Challenge *pending_challenges, int *nb_pending_challenges)
{
    refresh_challenges(pending_challenges, nb_pending_challenges);
    char buffer[BUF_SIZE];
    Client client = clients[client_idx];
    int challenge_idx =
        has_pending_challenge(client, pending_challenges, nb_pending_challenges);
    if (challenge_idx != -1)
    {
        Challenge challenge = pending_challenges[challenge_idx];
        if (strcmp(client.name, challenge.challenger.name) == 0)
        {
            // Le challenger a essayé de refuser le défi qu'il vient d'envoyer.
            write_client(
                client.sock,
                "Attendez que votre adversaire accepte/refuse le défi ou annulez-le, mais n'essayez pas de contourner le serveur...");
        }
        else
        {
            // Le client défié refuse le défi.
            send_challenge_denied_message(challenge);
            cancel_challenge(client, pending_challenges, nb_pending_challenges);
        }
    }
    else
    {
        write_client(
            client.sock,
            "Vous n'avez aucune demande de défi pour le moment.");
    }
}

// Retourne l'index du défi si le client est dans l'un des défis en attente, -1 sinon
int has_pending_challenge(Client client, Challenge *pending_challenges,
                          int *nb_pending_challenges)
{
    for (int i = 0; i < *nb_pending_challenges; i++)
    {
        if (strcmp(pending_challenges[i].challenger.name, client.name) == 0 ||
            strcmp(pending_challenges[i].challenged.name, client.name) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Supprime un défi du tableau
void remove_challenge(Challenge *pending_challenges, int to_remove,
                      int *nb_pending_challenges)
{
    memmove(pending_challenges + to_remove, pending_challenges + to_remove + 1,
            (*nb_pending_challenges - to_remove - 1) * sizeof(Challenge));
    (*nb_pending_challenges)--;
}

// Annule un défi
void cancel_challenge(Client client, Challenge *pending_challenges,
                      int *nb_pending_challenges)
{
    int i =
        has_pending_challenge(client, pending_challenges, nb_pending_challenges);
    if (i != -1)
    {
        remove_challenge(pending_challenges, i, nb_pending_challenges);
        i--;
    }
}

// Rafraîchir les défis
void refresh_challenges(Challenge *pending_challenges,
                        int *nb_pending_challenges)
{
    for (int i = 0; i < *nb_pending_challenges; i++)
    {
        unsigned long elapsed_time = difftime(time(NULL), pending_challenges[i].timestamp);
        // Si au moins un client est hors ligne ou que le timestamp dépasse le temps d'expiration
        if (elapsed_time > MAX_PENDING_TIME || pending_challenges[i].challenger.status == OFFLINE ||
            pending_challenges[i].challenged.status == OFFLINE)
        {
            if (elapsed_time > MAX_PENDING_TIME)
            {
                // Notifier les deux clients que le défi a expiré
                write_client(pending_challenges[i].challenger.sock, "La demande de défi a expiré.\n");
                write_client(pending_challenges[i].challenged.sock, "La demande de défi a expiré.\n");
            }
            // Puis le supprimer
            remove_challenge(pending_challenges, i, nb_pending_challenges);
            i--;
        }
    }
}
