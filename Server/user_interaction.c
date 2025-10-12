#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "client.h"
#include "utils.h"
#include "server.h"
#include "message.h"
#include "user_interaction.h"

void chat(const Client *clients, int actual, Client sender, const char *receiver_msg)
{
    // envoyer un chat de sender au destinataire

    // vérifie les paramètres de la commande
    if (*receiver_msg == '\0')
    {
        fwrite_client(sender.sock, "Nom et message manquants ! Syntaxe : chat [pseudo] [msg]");
        return;
    }

    // extrait le nom du destinataire et le message dans la commande
    char receiver[NAME_SIZE];
    const char *msg = extract_first_word(receiver_msg, receiver, NAME_SIZE) + 1;

    if (*msg == '\0')
    {
        fwrite_client(sender.sock, "Message introuvable ! Syntaxe : chat [pseudo] [msg]");
        return;
    }

    // vérifie si le pseudo existe
    int receiver_client_idx = name_exists(clients, actual, receiver);
    if (receiver_client_idx == -1)
    {
        fwrite_client(
            sender.sock,
            "Ce pseudo n'existe pas");
        return;
    }

    // vérifie le statut du client
    if (clients[receiver_client_idx].status == OFFLINE)
    {
        fwrite_client(
            sender.sock,
            "%s est hors ligne ! Il ne peut pas recevoir le message",
            clients[receiver_client_idx].name);
        return;
    }

    fwrite_client(
        clients[receiver_client_idx].sock,
        "%s : %s",
        sender.name, msg);
}

void set_bio(Client *ptr_client, const char *bio)
{
    // définit la bio du client
    strncpy(ptr_client->bio, bio, BIO_SIZE - 1);
    ptr_client->bio[BIO_SIZE - 1] = '\0';
}

void show_bio(const Client *clients, int actual, Client sender, const char *name)
{
    // envoyer la bio de name au sender

    // vérifie les paramètres de la commande
    if (*name == '\0')
    {
        fwrite_client(
            sender.sock,
            "Nom introuvable ! Syntaxe : bio [pseudo]");
        return;
    }

    // vérifie si le pseudo existe
    int client_idx = name_exists(clients, actual, name);
    if (client_idx != -1)
    {
        fwrite_client(
            sender.sock,
            "Bio de %s : %s",
            clients[client_idx].name,
            *clients[client_idx].bio == '\0'
                ? "Aucune bio ajoutée pour l'instant !"
                : clients[client_idx].bio);
    }
    else
    {
        fwrite_client(
            sender.sock,
            "Ce pseudo n'existe pas");
    }
}

int make_friend(Client *clients, int actual, int ind_sender, const char *name_requested)
{
    // envoyer une demande d'ami

    // vérifie les paramètres de la commande
    if (*name_requested == '\0')
    {
        fwrite_client(clients[ind_sender].sock,
                      "Nom introuvable ! Syntaxe : make friend [pseudo]");
        return -1;
    }

    // vérifie si on envoie une demande à soi-même
    if (strcmp(clients[ind_sender].name, name_requested) == 0)
    {
        fwrite_client(clients[ind_sender].sock, "Vous ne pouvez pas vous envoyer une demande d'ami !");
        return -1;
    }

    // vérifie si le joueur existe
    int client_requested_id = name_exists(clients, actual, name_requested);
    if (client_requested_id == -1)
    {
        fwrite_client(clients[ind_sender].sock, "Le joueur %s n'existe pas !", name_requested);
        return -1;
    }

    // check if already friends
    Client sender = clients[ind_sender];
    Client requested = clients[client_requested_id];
    int ind_fr_sender = find_fr_req(sender, name_requested);

    // cas existant
    if (ind_fr_sender != -1)
    {
        Friend_Req fr = sender.friend_req[ind_fr_sender];
        switch (fr.status)
        {
        case ACCEPTED:
            fwrite_client(sender.sock, "Vous et %s êtes déjà amis !", name_requested);
            return -1;
        case PENDING:
            fwrite_client(sender.sock, "Une demande d'ami entre vous existe déjà, veuillez accepter ou refuser !");
            return -1;
        case DENIED:
            fwrite_client(sender.sock, "Une demande entre vous a été refusée une fois ! Une autre demande sera envoyée !");
            break;
        default:
            break;
        }
    }

    // create friends requests if not exist
    if (requested.friend_req == NULL)
    {
        clients[client_requested_id].friend_req = malloc(MAX_FRIEND_REQ * sizeof(Friend_Req));
    }
    if (sender.friend_req == NULL)
    {
        clients[ind_sender].friend_req = malloc(MAX_FRIEND_REQ * sizeof(Friend_Req));
    }

    // create friend requests and send message to them
    int ind_fr_requested = find_fr_req(requested, sender.name);
    pending_friend_request(&clients[ind_sender], ind_fr_sender, &clients[client_requested_id], ind_fr_requested);
    send_msg_friend_request(sender, requested);

    return client_requested_id;
}

void pending_friend_request(Client *ptr_sender, int ind_fr_sender, Client *ptr_requested, int ind_fr_requested)
{
    // update friend request of sender and requested at the index of friends requests correspondant
    // case ind_fr = -1 means create new friend req

    int id_fr_sender = ind_fr_sender == -1
                           ? ptr_sender->nb_friend_req
                           : ind_fr_sender;
    int id_fr_requested = ind_fr_requested == -1
                              ? ptr_requested->nb_friend_req
                              : ind_fr_requested;

    // update friend request for sender
    strcpy((ptr_sender->friend_req[id_fr_sender]).name_client, ptr_requested->name);
    (ptr_sender->friend_req[id_fr_sender]).status = PENDING;
    (ptr_sender->friend_req[id_fr_sender]).is_sender = false;

    // Update friend request for client requested
    strcpy((ptr_requested->friend_req[id_fr_requested]).name_client, ptr_sender->name);
    (ptr_requested->friend_req[id_fr_requested]).status = PENDING;
    (ptr_requested->friend_req[id_fr_requested]).is_sender = true;

    // case create new friend request, nb_fr need to update
    if (ind_fr_sender == -1)
    {
        ptr_sender->nb_friend_req++;
    }
    if (ind_fr_requested == -1)
    {
        ptr_requested->nb_friend_req++;
    }
}

int accept_friend_req(Client acceptor, const char *name, Client clients[], int actual)
{
    // accepter une demande d'ami

    // vérifie les paramètres de la commande
    if (*name == '\0')
    {
        fwrite_client(acceptor.sock,
                      "Nom introuvable ! Syntaxe : accept fr [pseudo]");
        return -1;
    }

    // check if exist fr from name
    int ind_fr_acceptor = find_fr_req(acceptor, name);

    // case not exist
    if (ind_fr_acceptor == -1)
    {
        fwrite_client(acceptor.sock, "Vous n'avez pas de demande d'ami de %s !", name);
        return -1;
    }
    Friend_Req fr = acceptor.friend_req[ind_fr_acceptor];

    // case acceptor is the sender of friend req, so can not accept
    if (!fr.is_sender && fr.status != DELETED)
    {
        fwrite_client(acceptor.sock, "Vous êtes celui qui a envoyé la demande d'ami, vous ne pouvez donc pas l'accepter ! Annulez-la si vous le souhaitez !");
        return -1;
    }

    // handle different cases of status
    switch (fr.status)
    {
    case ACCEPTED:
        fwrite_client(acceptor.sock, "Vous avez déjà accepté la demande d'ami !");
        return -1;
    case DENIED:
        fwrite_client(acceptor.sock, "Vous avez déjà refusé la demande d'ami. Envoyez une nouvelle demande avec 'make friend' !");
        return -1;
    case DELETED:
        fwrite_client(acceptor.sock, "Vous n'avez pas de demande d'ami de %s !", name);
        return -1;
    default:
        break;
    }

    // case exist: update 2 friend request both 2 sides
    acceptor.friend_req[ind_fr_acceptor].status = ACCEPTED;
    int ind_sender = name_exists(clients, actual, acceptor.friend_req[ind_fr_acceptor].name_client);
    int ind_fr_sender = find_fr_req(clients[ind_sender], acceptor.name);
    clients[ind_sender].friend_req[ind_fr_sender].status = ACCEPTED;

    fwrite_client(acceptor.sock, "Vous et %s êtes maintenant amis !", clients[ind_sender].name);
    if (clients[ind_sender].status != OFFLINE)
    {
        fwrite_client(clients[ind_sender].sock, "%s a accepté votre demande d'ami !", acceptor.name);
    }
    return ind_sender;
}

int deny_friend_request(Client refuser, const char *name, Client clients[], int actual)
{
    // refuser une demande d'ami

    // vérifie les paramètres de la commande
    if (*name == '\0')
    {
        fwrite_client(refuser.sock,
                      "Nom introuvable ! Syntaxe : deny fr [pseudo]");
        return -1;
    }

    // check if exist fr from name
    int ind_fr_refuser = find_fr_req(refuser, name);

    // case not exist
    if (ind_fr_refuser == -1)
    {
        fwrite_client(refuser.sock, "Vous n'avez pas de demande d'ami de %s !", name);
        return -1;
    }
    Friend_Req fr = refuser.friend_req[ind_fr_refuser];

    // case refuser is the sender of friend req, so can not deny
    if (!fr.is_sender && fr.status != DELETED)
    {
        fwrite_client(refuser.sock, "Vous êtes celui qui a envoyé la demande d'ami, vous ne pouvez donc pas refuser ! Annulez-la à la place !");
        return -1;
    }

    // handle different cases of status
    switch (fr.status)
    {
    case ACCEPTED:
        fwrite_client(refuser.sock, "Vous avez déjà accepté la demande d'ami ! Veuillez plutôt supprimer l'ami !");
        return -1;
    case DENIED:
        fwrite_client(refuser.sock, "Vous avez déjà refusé la demande d'ami. Pas besoin de refuser à nouveau.");
        return -1;
    case DELETED:
        fwrite_client(refuser.sock, "Vous n'avez pas de demande d'ami de %s !", name);
        return -1;
    default:
        break;
    }

    // case exist: update 2 friend request both 2 sides
    refuser.friend_req[ind_fr_refuser].status = DENIED;
    int ind_sender = name_exists(clients, actual, refuser.friend_req[ind_fr_refuser].name_client);
    int ind_fr_sender = find_fr_req(clients[ind_sender], refuser.name);
    clients[ind_sender].friend_req[ind_fr_sender].status = DENIED;

    fwrite_client(refuser.sock, "Demande d'ami refusée avec succès !", clients[ind_sender].name);
    if (clients[ind_sender].status != OFFLINE)
    {
        fwrite_client(clients[ind_sender].sock, "%s a refusé votre demande d'ami !", refuser.name);
    }

    return ind_sender;
}

int cancel_friend_request(Client refuser, const char *name, Client clients[], int actual)
{
    // annuler une demande d'ami

    // vérifie les paramètres de la commande
    if (*name == '\0')
    {
        fwrite_client(refuser.sock,
                      "Nom introuvable ! Syntaxe : cancel fr [pseudo]");
        return -1;
    }

    // check if exist fr from name
    int ind_fr_refuser = find_fr_req(refuser, name);

    // case not exist
    if (ind_fr_refuser == -1)
    {
        fwrite_client(refuser.sock, "Vous n'avez pas envoyé de demande d'ami à %s !", name);
        return -1;
    }
    Friend_Req fr = refuser.friend_req[ind_fr_refuser];

    // case refuser is the sender of friend req, so can not deny
    if (fr.is_sender && fr.status != DELETED)
    {
        fwrite_client(refuser.sock, "Vous n'êtes pas celui qui a envoyé la demande d'ami, vous ne pouvez donc pas l'annuler ! Refusez-la à la place !");
        return -1;
    }

    // handle different cases of status
    switch (fr.status)
    {
    case ACCEPTED:
        fwrite_client(refuser.sock, "%s a déjà accepté la demande d'ami ! Veuillez plutôt supprimer l'ami !", name);
        return -1;
    case DENIED:
        fwrite_client(refuser.sock, "%s a déjà refusé la demande d'ami.", name);
        return -1;
    case DELETED:
        fwrite_client(refuser.sock, "Vous n'avez pas de demande d'ami de %s !", name);
        return -1;
    default:
        break;
    }

    // case exist: update 2 friend request both 2 sides
    refuser.friend_req[ind_fr_refuser].status = DELETED;
    int ind_sender = name_exists(clients, actual, refuser.friend_req[ind_fr_refuser].name_client);
    int ind_fr_sender = find_fr_req(clients[ind_sender], refuser.name);
    clients[ind_sender].friend_req[ind_fr_sender].status = DELETED;

    fwrite_client(refuser.sock, "Vous avez annulé votre demande d'ami à %s !", name);
    return ind_sender;
}

int unfriend(Client demander, const char *name, Client clients[], int actual)
{
    // retirer un ami

    // vérifie les paramètres de la commande
    if (*name == '\0')
    {
        fwrite_client(demander.sock,
                      "Nom introuvable ! Syntaxe : unfriend [pseudo]");
        return -1;
    }

    // check if exist fr from name
    int ind_fr_demander = find_fr_req(demander, name);

    // case not exist
    if (ind_fr_demander == -1)
    {
        fwrite_client(demander.sock, "Vous n'avez pas envoyé de demande d'ami à %s !", name);
        return -1;
    }
    Friend_Req fr = demander.friend_req[ind_fr_demander];

    // si la demande d'ami est déjà supprimée
    if (fr.status == DELETED)
    {
        fwrite_client(demander.sock, "Vous et %s n'êtes pas amis, pas de soucis !", name);
        return -1;
    }

    // handle different cases of status
    switch (fr.status)
    {
    case DENIED:
        fwrite_client(demander.sock, "%s a déjà accepté la demande d'ami auparavant !", fr.is_sender ? "Vous" : name);
        return -1;
    case PENDING:
        fwrite_client(demander.sock, "Il existe une demande d'ami entre vous ! Utilisez %s à la place !", fr.is_sender ? "deny fr" : "cancel fr");
        return -1;
    default:
        break;
    }

    // case exist: update 2 friend request both 2 sides
    demander.friend_req[ind_fr_demander].status = DELETED;
    int ind_sender = name_exists(clients, actual, demander.friend_req[ind_fr_demander].name_client);
    int ind_fr_sender = find_fr_req(clients[ind_sender], demander.name);
    clients[ind_sender].friend_req[ind_fr_sender].status = DELETED;

    fwrite_client(demander.sock, "Vous et %s n'êtes plus amis !", name);
    return ind_sender;
}

int find_fr_req(Client client, const char *name_to_find)
{
    // recherche dans les demandes d'ami du client si une demande existe pour name_to_find

    for (int i = 0; i < client.nb_friend_req; i++)
    {
        if (strcmp(name_to_find, (client.friend_req[i]).name_client) == 0)
        {
            return i;
        }
    }
    return -1;
}

int is_friend(Client client, const char *name)
{
    int ind_fr_req = find_fr_req(client, name);
    if (ind_fr_req == -1 || client.friend_req[ind_fr_req].status == ACCEPTED)
    {
        return ind_fr_req;
    }
    return -1;
}