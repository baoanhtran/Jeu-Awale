#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h>  /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#include <stdbool.h>

#define NAME_SIZE 256
#define BIO_SIZE 1024
#define MAX_FRIEND_REQ 200

typedef enum
{
    ONLINE,
    OFFLINE,
    IN_GAME,
    OBSERVING,
} Status;

typedef enum
{
    PENDING,
    ACCEPTED,
    DENIED,
    DELETED,         // la demande d'ami a été supprimée (par l'envoyeur)
} Friend_Req_Status;

typedef struct Client Client;
typedef struct Friend_Req Friend_Req;

struct Friend_Req
{
    char name_client[NAME_SIZE];
    Friend_Req_Status status;
    bool is_sender; // vrai si le client est l'envoyeur de la demande d'ami
};

struct Client
{
    SOCKET sock;
    Status status;
    char ip[INET_ADDRSTRLEN];
    char name[NAME_SIZE];
    char bio[BIO_SIZE];
    Friend_Req *friend_req;
    int nb_friend_req;
    int elo_scores;
};

#endif /* guard */
