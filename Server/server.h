#ifndef SERVER_H
#define SERVER_H

#ifdef WIN32

#include <winsock2.h>

#elif defined (linux) || defined (__APPLE__) || defined (__MACH__) || defined(__LINUX__)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else

#error not defined for this platform

#endif

#include "persistence.h"
#include "challenge.h"
#include "game.h"
#include "client.h"

#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100
#define BUF_SIZE    2048


// commands
static void handle_commands( 
	char* command,
	Client *clients, int actual, int id_sender ,
	Challenge* pending_challenges, int* pending_challenges_nb,
	Game* games, int* games_nb
); //handle command inputed of client
static void command_not_found(Client sender, const char * command); //handle case inputed command is not in list of commands

// Apps
static void init(void);
static void end(void);
static void app(void); 

//connections
static int init_connection(void);
static void end_connection(int sock);
static void create_client(Client *clients, int *actual, int csock, int *max, fd_set *rdfs, const char *client_ip, const char *name); // create a client
static void disconnect_client(Client *clients, int to_remove, int actual, Game *games, int *nb_games);
static void connect_client(Client *clients, int *actual, int csock, int *max, fd_set *rdfs, const char *client_ip, const char *name, Game *games, int nb_games);
static void reconnect_client_in_game(Game *games, int game_idx, SOCKET sock);

//communications
static int read_client(SOCKET sock, char *buffer);
void write_client(SOCKET sock, const char *buffer);
void fwrite_client(SOCKET sock, const char *format, ...); //combination of write_client() and printf()
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void send_connected_names(Client *connected_clients, SOCKET sock, int actual);

//data
static void init_client(Client *ptr, SOCKET sock, const char* ip, const char *name ); //init every attributes of client
int name_exists(const Client *clients, int actual, const char *name);
static void clear_clients(Client *clients, int actual); //close socket of clients and free all malloc for arrays
static void clear_game(Game *games, int nb_games); //free all malloc for arrays in game
static void clear_challenge(Challenge *challenges, int nb_challenges); //free all malloc for arrays in game

#endif /* guard */
