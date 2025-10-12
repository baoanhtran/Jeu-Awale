#include <string.h>

#include "message.h"
#include "server.h"
#include "user_interaction.h"

char buffer[BUF_SIZE];

void send_menu_commands(SOCKET sock)
{
    // affiche toutes les commandes du jeu à un client
    write_client(
        sock,
        "Liste des commandes :\n"
        "  - menu : voir toutes les commandes.\n"
        "  - names : voir tous les clients connectés et leur statut.\n"
        "  - rank : afficher le top 50 des joueurs.\n"
        "  - challenge [pseudo] : défier un client. Jouer avec les commandes :\n"
        "       + play [numéro_case] : jouer depuis la case numéro.\n"
        "  - games : voir toutes les parties en cours.\n"
        "  - set visibility [private/public] : définir la visibilité de la partie en cours.\n"
        "  - observe [pseudo] : participer à la partie d'un utilisateur en tant qu'observateur.\n"
        "  - leave : quitter le mode observation.\n"
        "  - chat [pseudo] [message] : envoyer un message privé à un client.\n"
        "  - set bio [description] : définir votre biographie.\n"
        "  - bio [pseudo] : voir la biographie d'un client.\n"
        "  - clear : nettoyer le terminal.\n"
        "  - make friend [pseudo] : envoyer une demande d'ami.\n"
        "       + accept fr [pseudo] : accepter une demande d'ami.\n"
        "       + deny fr [pseudo] : refuser une demande d'ami.\n"
        "       + cancel fr [pseudo] : annuler une demande d'ami envoyée.\n"
        "  - unfriend [pseudo] : supprimer un ami.\n"
        "  - friends : afficher la liste de mes amis.\n"
        "  - friend requests : afficher mes demandes d'ami en attente.\n");
}

void send_challenge_message(Client challenger, Client challenged)
{
    switch (challenged.status)
    {
    case OFFLINE:
        write_client(challenger.sock, "Ce client est hors ligne.");
        break;
    case IN_GAME:
        write_client(challenger.sock, "Ce client est déjà en partie.");
        break;
    case OBSERVING:
        write_client(challenger.sock, "Ce client regarde actuellement une partie.");
        break;
    case ONLINE:
        strncpy(buffer, challenger.name, BUF_SIZE - 1);
        strncat(buffer, " vous a défié. [accept/deny]",
                BUF_SIZE - strlen(buffer) - 1);
        write_client(challenged.sock, buffer);
        write_client(challenger.sock, "Demande de défi envoyée.");
        break;
    }
}

void send_received_challenge(Challenge challenge)
{
    strncpy(buffer, challenge.challenger.name, BUF_SIZE - 1);
    strncat(buffer, " vous a déjà envoyé une demande de défi. [accept/deny] avant d'en envoyer un autre.", BUF_SIZE - strlen(buffer) - 1);
    write_client(challenge.challenged.sock, buffer);
}

void send_challenge_accepted_message(Challenge challenge)
{
    strncpy(buffer, challenge.challenged.name, BUF_SIZE - 1);
    strncat(buffer, " a accepté le défi. Rejoindre la partie...",
            BUF_SIZE - strlen(buffer) - 1);
    write_client(challenge.challenger.sock, buffer);
    strncpy(buffer, "Défi accepté. Rejoindre la partie...", BUF_SIZE - 1);
    write_client(challenge.challenged.sock, buffer);
}

void send_challenge_denied_message(Challenge challenge)
{
    strncpy(buffer, challenge.challenged.name, BUF_SIZE - 1);
    strncat(buffer, " a refusé le défi.",
            BUF_SIZE - strlen(buffer) - 1);
    write_client(challenge.challenger.sock, buffer);
    write_client(challenge.challenged.sock, "Défi refusé.");
}

void send_to_all_observators(Game game, const char *buffer)
{
    // envoie buffer à tous les observateurs de la partie
    for (int i = 0; i < game.nb_observators; i++)
    {
        Client obs = *(game.ptr_observators[i]);
        if (obs.status == OBSERVING)
        {
            write_client(obs.sock, buffer);
        }
    }
}

void send_msg_friend_request(Client sender, Client requested)
{
    // envoie le contenu d'une demande d'ami au sender et au requested
    fwrite_client(sender.sock, "Demande d'ami envoyée !");
    if (requested.status != OFFLINE)
    {
        fwrite_client(requested.sock, "%s vous a envoyé une demande d'ami ! [accept fr %s / deny fr %s] Vous pouvez l'ignorer et accepter plus tard.", sender.name, sender.name, sender.name);
    }
}

void send_list_friend_request(Client sender)
{
    // envoie la liste des demandes d'ami au sender

    // vérifie si le sender a des demandes
    if (sender.nb_friend_req == 0)
    {
        write_client(sender.sock, "Vous n'avez aucune demande d'ami pour l'instant !");
        return;
    }

    // compte les demandes non supprimées
    int count = 0;
    for (int i = 0; i < sender.nb_friend_req; i++)
    {
        if (sender.friend_req[i].status != DELETED)
        {
            count++;
        }
    }

    if (count == 0)
    {
        write_client(sender.sock, "Vous n'avez aucune demande d'ami pour l'instant !");
        return;
    }

    // envoie la liste
    char buffer[BUF_SIZE];
    sprintf(buffer, "Vous avez %d demandes d'ami : \n", count);
    for (int i = 0; i < sender.nb_friend_req; i++)
    {
        Friend_Req fr = sender.friend_req[i];

        // convertit l'enum en chaîne
        char status[11];
        switch (fr.status)
        {
        case ACCEPTED:
            strcpy(status, "ACCEPTÉ");
            break;
        case DENIED:
            strcpy(status, "REFUSÉ");
            break;
        case PENDING:
            strcpy(status, "EN ATTENTE");
            break;
        default:
            strcpy(status, "SUPPRIMÉ");
            break;
        }
        char info[BUF_SIZE];
        if (fr.status != DELETED)
        { // ne pas afficher les demandes supprimées
            sprintf(info,
                    "  - %s [%s]\n",
                    fr.name_client, status);
            strncat(buffer, info, BUF_SIZE - 1);
        }
    }
    write_client(sender.sock, buffer);
}

void send_list_friend(Client sender, Client clients[], int actual)
{
    // envoie la liste d'amis du sender

    // vérifie si le sender a des amis
    if (sender.nb_friend_req == 0)
    {
        write_client(sender.sock, "Vous n'avez pas encore d'amis. Envoyez une demande !");
        return;
    }

    // compte les amis
    int count = 0;
    for (int i = 0; i < sender.nb_friend_req; i++)
    {
        if (sender.friend_req[i].status == ACCEPTED)
        {
            count++;
        }
    }

    if (count == 0)
    {
        write_client(sender.sock, "Vous n'avez pas encore d'amis. Envoyez une demande !");
        return;
    }

    // envoie la liste
    char buffer[BUF_SIZE];
    sprintf(buffer, "Vous avez %d amis : \n", count);

    for (int i = 0; i < sender.nb_friend_req; i++)
    {
        Friend_Req fr = sender.friend_req[i];
        int ind_client = name_exists(clients, actual, fr.name_client);

        // convertit l'enum en chaîne
        char info[BUF_SIZE];
        if (fr.status == ACCEPTED)
        {
            char status[11];
            switch (clients[ind_client].status)
            {
            case ONLINE:
                strcpy(status, "EN LIGNE");
                break;
            case OFFLINE:
                strcpy(status, "HORS LIGNE");
                break;
            case OBSERVING:
                strcpy(status, "OBSERVANT");
                break;
            case IN_GAME:
                strcpy(status, "EN PARTIE");
                break;
            }

            sprintf(info, "  - [%s] %s\n", status, fr.name_client);
            strncat(buffer, info, BUF_SIZE - 1);
        }
    }
    write_client(sender.sock, buffer);
}
