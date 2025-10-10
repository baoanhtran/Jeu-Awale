#ifndef CLIENT_H
#define CLIENT_H

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

#include <stdbool.h>

#define NAME_SIZE   256
#define BIO_SIZE    1024
#define MAX_FRIEND_REQ 200

typedef enum {
	ONLINE,
	OFFLINE,
	IN_GAME,
	OBSERVING,
} Status; // To know what is the current state of our client

typedef enum {
	PENDING,
    ACCEPTED,
    DENIED,
    DELETED, //sender cancel the FR
} Friend_Req_Status;  // For the friends feature

// Forward declaration of Client
typedef struct Client Client; //to make Friend Req can see Client
typedef struct Friend_Req Friend_Req;

struct Friend_Req {
    char name_client[NAME_SIZE];
    Friend_Req_Status status;
    bool is_sender; //is the client in Friend Req is the sender of friend request
};

struct Client {
    SOCKET sock;
    Status status; 
    char ip[INET_ADDRSTRLEN];
    char name[NAME_SIZE];
    char bio[BIO_SIZE];
    Friend_Req *friend_req; //TODO: change name to friend_reqs if possible
    int nb_friend_req;
    int elo_scores;
};

#endif /* guard */
