#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>

#include "utils.h"
#include "client.h"
#include "game.h"
#include "challenge.h"
#include "message.h"
#include "user_interaction.h"
#include "ranking.h"
#include "server.h"

#define MAX_PENDING_CHALLENGES MAX_CLIENTS / 2
#define MAX_GAMES MAX_CLIENTS / 2

// #########################################################
//  						COMMANDES
// #########################################################

static void handle_commands(
    char *command,
    Client *clients, int actual, int id_sender,
    Challenge *pending_challenges, int *pending_challenges_nb,
    Game *games, int *games_nb)
{
    // gérer la commande saisie par le client
    bool print_separator = true;

    //====================== COMMANDES DE BASE ============================

    if (strcmp(command, "names") == 0)
    {
        send_connected_names(clients, clients[id_sender].sock, actual);
    }
    else if (strcmp(command, "menu") == 0)
    {
        send_menu_commands(clients[id_sender].sock);
    }
    else if (strcmp(command, "rank") == 0)
    {
        display_sorted_players(clients[id_sender], clients, actual);

        //====================== COMMANDES D'AMITIÉ ============================
    }
    else if (strncmp(command, "make friend ", 12) == 0)
    {
        const char *name = command + 12;
        const int ind_requested = make_friend(clients, actual, id_sender, name);
        // persistance du client pour la demande d'ami
        if (ind_requested != -1)
        {
            updateClient(clients[ind_requested]);
            updateClient(clients[id_sender]);
        }
    }
    else if (strncmp(command, "accept fr ", 10) == 0)
    {
        const char *name = command + 10;
        const int ind_fr_sender = accept_friend_req(clients[id_sender], name, clients, actual);
        // persistance du client pour la demande d'ami
        if (ind_fr_sender != -1)
        {
            updateClient(clients[ind_fr_sender]);
            updateClient(clients[id_sender]);
        }
    }
    else if (strncmp(command, "deny fr ", 8) == 0)
    {
        const char *name = command + 8;
        const int ind_fr_sender = deny_friend_request(clients[id_sender], name, clients, actual);
        // persistance du client pour la demande d'ami
        if (ind_fr_sender != -1)
        {
            updateClient(clients[ind_fr_sender]);
            updateClient(clients[id_sender]);
        }
    }
    else if (strncmp(command, "cancel fr ", 10) == 0)
    {
        const char *name = command + 10;
        const int ind_requested = cancel_friend_request(clients[id_sender], name, clients, actual);
        // persistance du client pour la demande d'ami
        if (ind_requested != -1)
        {
            updateClient(clients[ind_requested]);
            updateClient(clients[id_sender]);
        }
    }
    else if (strncmp(command, "unfriend ", 9) == 0)
    {
        const char *name = command + 9;
        const int ind_opponent = unfriend(clients[id_sender], name, clients, actual);
        // persistance du client pour la suppression d'ami
        if (ind_opponent != -1)
        {
            updateClient(clients[ind_opponent]);
            updateClient(clients[id_sender]);
        }
    }
    else if (strncmp(command, "friends", 7) == 0)
    {
        send_list_friend(clients[id_sender], clients, actual);
    }
    else if (strncmp(command, "friend requests", 15) == 0)
    {
        send_list_friend_request(clients[id_sender]);

        //====================== COMMANDES DE DÉFI ============================
    }
    else if (strncmp(command, "challenge ", 10) == 0)
    {
        const char *name = command + 10;
        challenged(clients, actual, clients[id_sender], name, pending_challenges,
                   pending_challenges_nb);
    }
    else if (strncmp(command, "accept", 6) == 0)
    {
        accept_challenge(clients, actual, id_sender,
                         pending_challenges, pending_challenges_nb,
                         games, games_nb);
        print_separator = false;
    }
    else if (strncmp(command, "deny", 6) == 0)
    {
        deny_challenge(clients, id_sender, pending_challenges,
                       pending_challenges_nb);

        //====================== COMMANDES DE PARTIE ============================
    }
    else if (strncmp(command, "play ", 5) == 0)
    {
        const char *cell_str = command + 5;
        play(cell_str, clients[id_sender], games, *games_nb, clients, actual);
        print_separator = false;
    }
    else if (strncmp(command, "games", 5) == 0)
    {
        show_all_games(clients[id_sender], games, *games_nb);
    }
    else if (strncmp(command, "set visibility ", 15) == 0)
    {
        const char *visibility = command + 15;
        game_set_visiblity(clients[id_sender], visibility, games, *games_nb);
    }
    else if (strncmp(command, "observe ", 8) == 0)
    {
        const char *name = command + 8;
        observe_game(clients, actual, games, *games_nb, id_sender, name);
        if (clients[id_sender].status == OBSERVING)
        {
            print_separator = false;
        }
    }
    else if (strncmp(command, "leave", 5) == 0)
    {
        leave_observing(&clients[id_sender], games, *games_nb);

        //====================== MESSAGES - BIO ============================
    }
    else if (strncmp(command, "chat", 4) == 0)
    {
        const char *receiver_et_msg = command + 5;
        chat(clients, actual, clients[id_sender], receiver_et_msg);
    }
    else if (strncmp(command, "set bio ", 8) == 0)
    {
        const char *bio = command + 8;
        set_bio(clients + id_sender, bio);
        updateClient(clients[id_sender]);
    }
    else if (strncmp(command, "bio ", 4) == 0)
    {
        const char *name = command + 4;
        show_bio(clients, actual, clients[id_sender], name);

        //====================== AUTRES COMMANDES ============================
    }
    else
    {
        command_not_found(clients[id_sender], command);
    }

    if (print_separator)
    {
        write_client(clients[id_sender].sock, "\n============================================\n");
    }
}

static void command_not_found(Client sender, const char *command)
{
    // gérer le cas où la commande n'existe pas

    fwrite_client(sender.sock,
                  "'%s' : commande introuvable !\n",
                  command);
    send_menu_commands(sender.sock);
}

// #########################################################
//  						APPLICATION
// #########################################################

static void app(void)
{
    SOCKET sock = init_connection();
    char buffer[BUF_SIZE];

    /* un tableau pour tous les clients */
    int actual = 0;
    int max = sock;
    Client *clients = malloc(MAX_CLIENTS * sizeof(Client));
    /* persistance des clients */
    getClients(clients, &actual);

    int pending_challenges_nb = 0;
    Challenge pending_challenges[MAX_PENDING_CHALLENGES];

    /* un tableau pour les parties */
    int games_nb = 0;
    Game games[MAX_GAMES];

    fd_set rdfs;

    struct timeval timeout; // pour rafraîchir le serveur

    time_t last_refresh = time(NULL);
    while (1)
    {
        int i = 0;
        FD_ZERO(&rdfs);

        /* ajouter STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        /* ajouter le socket de connexion */
        FD_SET(sock, &rdfs);

        /* ajouter le socket de chaque client */
        for (i = 0; i < actual; i++)
        {
            if (clients[i].status != OFFLINE)
            {
                FD_SET(clients[i].sock, &rdfs);
            }
        }

        timeout.tv_sec = 1; // rafraîchissement chaque seconde
        timeout.tv_usec = 0;

        int activity = select(max + 1, &rdfs, NULL, NULL, &timeout);

        // Chaque seconde, rafraîchir les défis et les parties
        if (activity == 0)
        {
            refresh_challenges(pending_challenges, &pending_challenges_nb);
            refresh_paused_games(clients, actual, games, &games_nb);
            last_refresh = time(NULL);
            continue;
        }

        if (activity == -1)
        {
            perror("select()");
            exit(errno);
        }

        /* quelque chose depuis l'entrée standard : par ex. le clavier */
        if (FD_ISSET(STDIN_FILENO, &rdfs))
        {
            /* arrêter le processus lorsqu'on tape au clavier */
            break;
        }
        else if (FD_ISSET(sock, &rdfs))
        {
            SOCKADDR_IN csin = {0};
            socklen_t sinsize = sizeof csin;
            int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
            if (csock == SOCKET_ERROR)
            {
                perror("accept()");
                continue;
            }

            /* après connexion le client envoie son nom */
            if (read_client(csock, buffer) == -1)
            {
                /* déconnecté */
                continue;
            }

            // obtenir l'adresse IP du client
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &csin.sin_addr, client_ip, sizeof(client_ip));
            connect_client(clients, &actual, csock, &max, &rdfs, client_ip, buffer, games, games_nb);
        }
        else
        {
            int i = 0;
            for (i = 0; i < actual; i++)
            {
                // un client parle
                if (FD_ISSET(clients[i].sock, &rdfs) && clients[i].status != OFFLINE)
                {
                    Client client = clients[i];
                    int c = read_client(clients[i].sock, buffer);
                    // si le client est déconnecté on le déconnecte proprement et on annule son éventuel défi en attente
                    if (c == 0)
                    {
                        disconnect_client(clients, i, actual, games, &games_nb);
                        cancel_challenge(client, pending_challenges,
                                         &pending_challenges_nb);
                        FD_CLR(client.sock, &rdfs);
                    }
                    else
                    {
                        handle_commands(
                            buffer,
                            clients, actual, i,
                            pending_challenges, &pending_challenges_nb,
                            games, &games_nb);
                    }
                    break;
                }
            }
        }
    }

    clear_game(games, games_nb);
    clear_challenge(pending_challenges, pending_challenges_nb);
    clear_clients(clients, actual);
    end_connection(sock);
}

// #########################################################
//  						CONNEXIONS
// #########################################################

static int init_connection(void)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin = {0};

    if (sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }

    return sock;
}

static void end_connection(int sock) { closesocket(sock); }

// Créer un client (déclenché la première fois qu'un client se connecte)
static void create_client(Client *clients, int *actual, int csock, int *max, fd_set *rdfs, const char *client_ip, const char *name)
{
    init_client(&clients[*actual], csock, client_ip, name);
    // persistance du nouveau client
    addClient(clients[*actual]);

    fwrite_client(csock, "Bienvenue au jeu AWALE ! Amusez-vous bien ici avec nous !\n");
    send_menu_commands(csock);
    *max = csock > *max ? csock : *max;
    (*actual)++;
    FD_SET(csock, rdfs);
}

static void disconnect_client(Client *clients, int to_remove, int actual, Game *games, int *nb_games)
{
    // mettre à jour le statut, le socket et fermer le socket
    closesocket(clients[to_remove].sock);
    clients[to_remove].status = OFFLINE;
    clients[to_remove].sock = -1;
    int client_in_game = is_in_game(games, *nb_games, clients[to_remove]);
    // Le client est-il en jeu ?
    if (client_in_game != -1)
    {
        // Nous notifions son adversaire et les observateurs
        char buffer[BUF_SIZE];
        strcpy(buffer, clients[to_remove].name);
        strncat(buffer, " s'est déconnecté.\n\n", BUF_SIZE - strlen(buffer) - 1);
        send_to_all_observators(games[client_in_game], buffer); // notifie tous les observateurs

        bool challenged_is_disconnected = strcmp(games[client_in_game].client_challenged.name, clients[to_remove].name) == 0;
        // Mettre aussi à jour la copie du client (déconnecté) dans le tableau des parties
        if (challenged_is_disconnected)
        {
            games[client_in_game].client_challenged.status = OFFLINE;
        }
        else
        {
            games[client_in_game].client_challenger.status = OFFLINE;
        }

        // Les deux clients sont déconnectés : on termine la partie
        if (games[client_in_game].client_challenger.status == OFFLINE && games[client_in_game].client_challenged.status == OFFLINE)
        {
            send_to_all_observators(games[client_in_game], "Les deux joueurs sont maintenant déconnectés. La partie est annulée."); // notifie tous les observateurs
            delete_game(games, client_in_game, nb_games);
            return;
        }

        // Mettre la partie en état PAUSED
        games[client_in_game].game_state = PAUSED;
        int ind_challenger = name_exists(clients, actual, games[client_in_game].client_challenger.name);
        int ind_challenged = name_exists(clients, actual, games[client_in_game].client_challenged.name);
        // le joueur challengé s'est déconnecté
        if (challenged_is_disconnected)
        {
            // on met à jour son nombre de déconnexions et le timestamp
            games[client_in_game].game_disconnection->challenged_disconnections++;
            games[client_in_game].game_disconnection->elapsed_time = time(NULL);
            // si le client déconnecté s'est déconnecté trop de fois, on termine la partie
            if (games[client_in_game].game_disconnection->challenged_disconnections > MAX_DISCONNECTION)
            {
                strcpy(buffer, games[client_in_game].client_challenged.name);
                strncat(buffer, " s'est déconnecté trop de fois. ", BUF_SIZE - strlen(buffer) - 1);
                write_client(games[client_in_game].client_challenger.sock, buffer);
                write_client(games[client_in_game].client_challenger.sock, "Vous gagnez.");
                strncat(buffer, games[client_in_game].client_challenger.name, BUF_SIZE - strlen(buffer) - 1);
                strncat(buffer, " gagne.\nVous quittez maintenant le mode observation!\n", BUF_SIZE - strlen(buffer) - 1);
                send_to_all_observators(games[client_in_game], buffer);

                games[client_in_game].client_challenger.status = ONLINE;
                clients[ind_challenger].status = ONLINE;
                updateElo(&clients[ind_challenger], &clients[ind_challenged], WIN, K_COEF);
                // persistance du client pour le nouveau score ELO
                updateClient(clients[ind_challenger]);
                updateClient(clients[ind_challenged]);

                kick_all_observators(&games[client_in_game]);
                delete_game(games, client_in_game, nb_games);
                return;
            }
            else
            {
                // on notifie son adversaire d'attendre jusqu'à ce qu'il soit déclaré vainqueur
                write_client(games[client_in_game].client_challenger.sock, buffer);
                write_client(games[client_in_game].client_challenger.sock, "Si votre adversaire ne se reconnecte pas dans quelques secondes, vous serez déclaré gagnant.");
            }
        }
        else
        {
            // le challenger s'est déconnecté
            games[client_in_game].game_disconnection->challenger_disconnections++;
            games[client_in_game].game_disconnection->elapsed_time = time(NULL);
            if (games[client_in_game].game_disconnection->challenger_disconnections > MAX_DISCONNECTION)
            {
                strcpy(buffer, games[client_in_game].client_challenger.name);
                strncat(buffer, " s'est déconnecté trop de fois. ", BUF_SIZE - strlen(buffer) - 1);
                write_client(games[client_in_game].client_challenged.sock, buffer);
                write_client(games[client_in_game].client_challenged.sock, "Vous gagnez.");
                strncat(buffer, games[client_in_game].client_challenged.name, BUF_SIZE - strlen(buffer) - 1);
                strncat(buffer, " gagne.\nVous quittez maintenant le mode observation!\n", BUF_SIZE - strlen(buffer) - 1);
                send_to_all_observators(games[client_in_game], buffer);

                games[client_in_game].client_challenged.status = ONLINE;
                clients[name_exists(clients, actual, games[client_in_game].client_challenged.name)].status = ONLINE;
                updateElo(&clients[ind_challenger], &clients[ind_challenged], LOSE, K_COEF);
                // persistance du client pour le nouveau score ELO
                updateClient(clients[ind_challenger]);
                updateClient(clients[ind_challenged]);

                kick_all_observators(&games[client_in_game]);
                delete_game(games, client_in_game, nb_games);

                return;
            }
            else
            {
                write_client(games[client_in_game].client_challenged.sock, buffer);
                write_client(games[client_in_game].client_challenged.sock, "Si votre adversaire ne se reconnecte pas dans quelques secondes, vous serez déclaré gagnant.");
            }
        }
    }
}

static void connect_client(Client *clients, int *actual, int csock, int *max, fd_set *rdfs, const char *client_ip, const char *name, Game *games, int nb_games)
{
    int client_idx = name_exists(clients, *actual, name);
    // Le pseudo existe ?
    if (client_idx != -1)
    {
        Client client = clients[client_idx];
        // Le client voulant se connecter possède-t-il réellement ce pseudo ?
        if (strcmp(client.ip, client_ip) == 0)
        {
            if (client.status != OFFLINE)
            {
                // hôte déjà connecté avec ce pseudo
                write_client(csock, "déjà connecté");
                // Le client fermera le socket de son côté. Nous le détecterons ensuite dans app().
            }
            else
            {
                // reconnecte le client
                clients[client_idx].sock = csock;
                FD_SET(csock, rdfs);
                *max = csock > *max ? csock : *max;
                int client_in_game = is_in_game(games, nb_games, clients[client_idx]);
                // Si le client était en jeu, on le reconnecte
                if (client_in_game != -1)
                {
                    clients[client_idx].status = IN_GAME;
                    reconnect_client_in_game(games, client_in_game, csock);
                }
                else
                {
                    clients[client_idx].status = ONLINE;
                    fwrite_client(csock, "Bienvenue %s !\n", client.name);
                    send_menu_commands(csock);
                }
            }
        }
        else
        {
            // pseudo déjà pris par une autre adresse
            write_client(csock, "nom déjà pris");
        }
    }
    else
    {
        // ajoute un nouveau client
        create_client(clients, actual, csock, max, rdfs, client_ip, name);
        // fwrite_client(csock, "-%s-%lu", client_ip, strlen(client_ip));
    }
}

static void reconnect_client_in_game(Game *games, int game_idx, SOCKET sock)
{
    char buffer[BUF_SIZE];
    bool challenger_was_disconnected = games[game_idx].client_challenger.status == OFFLINE;
    // copie le bon nom de client dans le buffer
    challenger_was_disconnected ? strcpy(buffer, games[game_idx].client_challenger.name) : strcpy(buffer, games[game_idx].client_challenged.name);
    strncat(buffer, " s'est reconnecté.", BUF_SIZE - strlen(buffer) - 1);
    // notifie l'adversaire et les observateurs que le client s'est reconnecté
    challenger_was_disconnected ? write_client(games[game_idx].client_challenged.sock, buffer) : write_client(games[game_idx].client_challenger.sock, buffer);
    send_to_all_observators(games[game_idx], buffer); // notifie les observateurs
    // Comme nous n'utilisons pas de pointeurs dans nos structs, nous devons mettre à jour chaque copie du client...
    if (challenger_was_disconnected)
    {
        games[game_idx].client_challenger.sock = sock;
        games[game_idx].client_challenger.status = IN_GAME;
    }
    else
    {
        games[game_idx].client_challenged.sock = sock;
        games[game_idx].client_challenged.status = IN_GAME;
    }
    games[game_idx].game_state = IN_PROGRESS;
    display_game(games[game_idx]);
    display_turn(games[game_idx]);
}

// #########################################################
//  						COMMUNICATIONS
// #########################################################

static int read_client(SOCKET sock, char *buffer)
{
    int n = 0;

    if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
    {
        perror("recv()");
        /* si erreur recv on déconnecte le client */
        n = 0;
    }

    buffer[n] = 0;

    return n;
}

void write_client(SOCKET sock, const char *buffer)
{
    if (send(sock, buffer, strlen(buffer), 0) < 0)
    {
        perror("send()");
        exit(errno);
    }
}

void fwrite_client(SOCKET sock, const char *format, ...)
{
    char buffer[BUF_SIZE];

    // Use va_list to handle the variadic arguments
    va_list args;
    va_start(args, format);
    int written_size = vsnprintf(buffer, BUF_SIZE, format, args);
    va_end(args);

    // Check if the buffer was too small
    if (written_size >= BUF_SIZE)
    {
        // Truncate the string to fit the buffer
        buffer[BUF_SIZE - 1] = '\0';
    }

    write_client(sock, buffer);
}

static void send_message_to_all_clients(Client *clients, Client sender,
                                        int actual, const char *buffer,
                                        char from_server)
{
    int i = 0;
    char message[BUF_SIZE];
    message[0] = 0;
    for (i = 0; i < actual; i++)
    {
        /* on n'envoie pas le message à l'expéditeur */
        if (sender.sock != clients[i].sock && clients[i].status != OFFLINE)
        {
            if (from_server == 0)
            {
                strncpy(message, sender.name, BUF_SIZE - 1);
                strncat(message, " : ", sizeof message - strlen(message) - 1);
            }
            strncat(message, buffer, sizeof message - strlen(message) - 1);
            write_client(clients[i].sock, message);
        }
    }
}

static void send_connected_names(Client *connected_clients, SOCKET sock, int actual)
{
    char buffer[BUF_SIZE] = "\0";
    if (actual <= 1)
    {
        // Pas d'autres utilisateurs
        strncpy(buffer, "Vous êtes le seul ici...", BUF_SIZE - 1);
    }
    else
    {
        for (int i = 0; i < actual; i++)
        {
            Client c = connected_clients[i];
            char status[10];

            switch (c.status)
            {
            case ONLINE:
                strcpy(status, "EN_LIGNE");
                break;
            case OFFLINE:
                strcpy(status, "HORS_LIGNE");
                break;
            case OBSERVING:
                strcpy(status, "OBSERVATION");
                break;
            case IN_GAME:
                strcpy(status, "EN_JEU");
                break;
            }

            char info[NAME_SIZE + 32];
            snprintf(info, sizeof(info), "[%s] %s (score: %d)", status, c.name, c.elo_scores);
            strcat(buffer, info);
            if (c.sock == sock)
            {
                strcat(buffer, " <-- vous");
            }
            strcat(buffer, "\n");
        }
    }
    write_client(sock, buffer);
}

// #########################################################
//  						DONNÉES
// #########################################################

static void init_client(Client *ptr, SOCKET sock, const char *ip, const char *name)
{
    ptr->sock = sock;
    ptr->status = ONLINE;
    ptr->elo_scores = 1000;

    strncpy(ptr->ip, ip, INET_ADDRSTRLEN - 1);
    strncpy(ptr->name, name, NAME_SIZE - 1);
    ptr->name[NAME_SIZE - 1] = '\0';
    *ptr->bio = '\0';

    ptr->nb_friend_req = 0;
    ptr->friend_req = NULL;
}

// Retourne l'index du client qui a ce nom, -1 sinon
int name_exists(const Client *clients, int actual, const char *name)
{
    for (int i = 0; i < actual; i++)
    {
        if (strcmp(clients[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

static void clear_clients(Client *clients, int actual)
{
    // fermer les sockets des clients et libérer les malloc pour les tableaux

    for (int i = 0; i < actual; i++)
    {
        closesocket(clients[i].sock);
        free(clients[i].friend_req);
    }
    free(clients);
}

static void clear_game(Game *games, int nb_games)
{
    // libérer les malloc pour les tableaux dans Game
    for (int i = 0; i < nb_games; i++)
    {
        free(games[i].ptr_observators);
        free(games[i].partie);
    }
}

static void clear_challenge(Challenge *challenges, int nb_challenges)
{
    // libérer les malloc pour les tableaux dans Challenge
}

int main()
{
    app();

    return EXIT_SUCCESS;
}
