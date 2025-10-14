#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"

static void app(const char *address, const char *name)
{
    SOCKET sock = init_connection(address);
    char buffer[BUF_SIZE];

    fd_set rdfs;

    /* envoyer notre nom */
    write_server(sock, name);

    while (1)
    {
        FD_ZERO(&rdfs);

        /* ajouter STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        /* ajouter la socket */
        FD_SET(sock, &rdfs);

        if (select(sock + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }

        /* données depuis l'entrée standard : p.ex. le clavier */
        if (FD_ISSET(STDIN_FILENO, &rdfs))
        {
            if (fgets(buffer, BUF_SIZE - 1, stdin) == NULL)
            {
                // EOF détecté sur STDIN
                break;
            }

            char *p = NULL;
            p = strstr(buffer, "\n");
            if (p != NULL)
            {
                *p = 0;
            }
            else
            {
                /* nettoyage */
                buffer[BUF_SIZE - 1] = 0;
            }

            // Nettoie l'invite de commande
            if (strncmp(buffer, "clear", 5) == 0)
            {
                system("clear"); // effacer le terminal
            }
            else
            {
                write_server(sock, buffer);
            }
        }
        else if (FD_ISSET(sock, &rdfs))
        {
            int n = read_server(sock, buffer);
            /* serveur arrêté */
            if (n == 0)
            {
                printf("Serveur déconnecté !\n");
                break;
            }

            // Ce message est reçu si le client est déjà connecté avec le nom qu'il a envoyé.
            if (strncmp(buffer, "déjà connecté", strlen("déjà connecté")) == 0)
            {
                printf("Quelqu'un ayant la même IP que vous est déjà connecté à ce compte. Assurez-vous de ne pas avoir un autre terminal ouvert avec ce client.\n");
                break;
            }

            // Ce message est reçu si le nom du client est déjà utilisé.
            if (strncmp(buffer, "nom déjà pris", strlen("nom déjà pris")) == 0)
            {
                printf("Ce nom est déjà utilisé par un autre client. Essayez un autre nom.\n");
                break;
            }

            // NOTE : Dans les deux cas, le programme se termine

            puts(buffer);
        }
    }

    end_connection(sock);
}

static int init_connection(const char *address)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin = {0};
    struct hostent *hostinfo;

    if (sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }

    hostinfo = gethostbyname(address);
    if (hostinfo == NULL)
    {
        fprintf(stderr, "Hôte inconnu %s.\n", address);
        exit(EXIT_FAILURE);
    }

    sin.sin_addr = *(IN_ADDR *)hostinfo->h_addr_list[0];
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if (connect(sock, (SOCKADDR *)&sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect()");
        exit(errno);
    }

    return sock;
}

static void end_connection(int sock) { closesocket(sock); }

static int read_server(SOCKET sock, char *buffer)
{
    int n = 0;

    if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
    {
        perror("recv()");
        exit(errno);
    }

    buffer[n] = 0;

    return n;
}

static void write_server(SOCKET sock, const char *buffer)
{
    if (send(sock, buffer, strlen(buffer), 0) < 0)
    {
        perror("send()");
        exit(errno);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Utilisation : %s [adresse] [pseudo]\n", argv[0]);
        return EXIT_FAILURE;
    }

    app(argv[1], argv[2]);

    return EXIT_SUCCESS;
}
