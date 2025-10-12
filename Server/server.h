#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>     /* close */
#include <netdb.h>      /* gethostbyname */
#include <sys/select.h> /* fd_set */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#include "persistence.h"
#include "challenge.h"
#include "game.h"
#include "client.h"

#define CRLF "\r\n"
#define PORT 9998
#define MAX_CLIENTS 100
#define BUF_SIZE 2048

// commandes
static void handle_commands(
    char *command,
    Client *clients, int actual, int id_sender,
    Challenge *pending_challenges, int *pending_challenges_nb,
    Game *games, int *games_nb);                                   // gérer la commande envoyée par un client
static void command_not_found(Client sender, const char *command); // gérer le cas où la commande n'existe pas

// Apps
static void app(void);

// connexions
static int init_connection(void);
static void end_connection(int sock);
static void create_client(Client *clients, int *actual, int csock, int *max, fd_set *rdfs, const char *client_ip, const char *name); // créer un client
static void disconnect_client(Client *clients, int to_remove, int actual, Game *games, int *nb_games);
static void connect_client(Client *clients, int *actual, int csock, int *max, fd_set *rdfs, const char *client_ip, const char *name, Game *games, int nb_games);
static void reconnect_client_in_game(Game *games, int game_idx, SOCKET sock);

// communications
static int read_client(SOCKET sock, char *buffer);
void write_client(SOCKET sock, const char *buffer);
void fwrite_client(SOCKET sock, const char *format, ...); // combinaison de write_client() et printf()
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void send_connected_names(Client *connected_clients, SOCKET sock, int actual);

// données
static void init_client(Client *ptr, SOCKET sock, const char *ip, const char *name); // initialise tous les attributs d'un client
int name_exists(const Client *clients, int actual, const char *name);
static void clear_clients(Client *clients, int actual);                // ferme les sockets des clients et libère les malloc des tableaux
static void clear_game(Game *games, int nb_games);                     // libère les malloc dans les games
static void clear_challenge(Challenge *challenges, int nb_challenges); // libère les malloc dans les challenges

#endif /* guard */
