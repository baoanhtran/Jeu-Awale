#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "user_interaction.h"
#include "persistence.h"
#include "server.h"

void addClient(Client client) {
    // add a client to database

    FILE *file = fopen(FILENAME, "ab");
    if (file == NULL) {
        perror("Unable to open file");
        return;
    }

    file_write_client(&client, file);
    fclose(file);
}

void getClients(Client *clients, int *nb_clients) {
    // read all clients from database

    // open file
    FILE *file = fopen(FILENAME, "rb+");
    if (file == NULL) {
        if (fopen(FILENAME, "w") == NULL) {
            perror("Unable to open file");
            return;
        };
        fclose(file);
        file = fopen(FILENAME, "rb+");
    }

    // read clients from file and store them in clients[]
    int len = 0;
    while (!feof(file)) {

        if (file_read_client(&clients[len], file) == 0) break;
        clients[len].status = OFFLINE;
        clients[len].sock = -1;    
        len++;
    }
    *nb_clients = len;
    fclose(file);
}


void fetchClients(Client *clients, int *nb_clients) {
    //Cause db doesnt store STATUS and SOCK, so call getClients in middle of server will be crashed, use this instead
    //warning: cause this will  totally change the pointer and order of clients, so only use if you are sure that it will not affect the program

    // open file
    Client* _result = malloc( MAX_CLIENTS * sizeof(Client));
    int *len;

    // get clients from file, and update status and sock
    getClients(_result, len);
    for (int i = 0; i < *len; i++) {
        //update clients status and sock
        for (int j = 0; j < *nb_clients; j++) {
            if (strcmp(_result[i].name, clients[j].name) == 0) {
                _result[i].status = clients[j].status;
                _result[i].sock = clients[j].sock;
                break;
            }
        }
    }

    // free old clients[]
    free(clients);
    for (int i = 0; i < *len; i++) {
        free_client(&clients[i]);
    }

    *nb_clients = *len;
    clients = _result;
}

void updateClient(Client newClient) {
    // update a client in database, client is identified by name

    // open file
    FILE *file = fopen(FILENAME, "rb");
    FILE *tempFile = fopen("__temp.dat", "wb");
    if (file == NULL || tempFile == NULL) {
        perror("Unable to open file");
        return;
    }
    const char *name = newClient.name;
    
    // store all clients and newClient in tempFile
    while (!feof(file)) {
        Client client;
        if (file_read_client(&client, file) == 0) break;

        if (strcmp(client.name, name) == 0) {
            file_write_client(&newClient, tempFile);
        } else {
            file_write_client(&client, tempFile);
        }
        free_client(&client);
    }

    fclose(file);
    fclose(tempFile);

    // change tempFile back to original
    remove(FILENAME);
    rename("__temp.dat", FILENAME);
}

void deleteClient(const char *name) {
    // delete a client from database, client is identified by name

    // open file
    FILE *file = fopen(FILENAME, "rb");
    FILE *tempFile = fopen("_temp.dat", "wb");
    if (file == NULL || tempFile == NULL) {
        perror("Unable to open file");
        return;
    }

    // store all clients except the one with name in tempFile
    while (1) {
        Client client;
        
        file_read_client(&client, file);
        if (strcmp(client.name, name) != 0) {
            file_write_client(&client, file);
        }
        free_client(&client);
    }
    fclose(file);
    fclose(tempFile);

    // change tempFile back to original
    remove(FILENAME);
    rename("_temp.dat", FILENAME);
}

size_t fwrite_string(const char *str, FILE *file) {
    // write string to file

    size_t len = strlen(str) + 1;
    fwrite(&len, sizeof(size_t), 1, file);
    return fwrite(str, sizeof(char), len, file);
}

size_t fread_string(char *ptr_buffer, FILE *file) {
    // read string from file (string has been added by fwrite_string)
    size_t len;
    if (fread(&len, sizeof(size_t), 1, file) != 1) return 0;
    size_t len_read = fread(ptr_buffer, sizeof(char), len, file);

    if (len_read != len) return 0;
    return len_read;
}

size_t file_write_client(const Client *client, FILE *file) {
    // write a client to file

    size_t len = 0;
    // No need to persist socket and status
    len += fwrite(&client->nb_friend_req, sizeof(int), 1, file);
    len += fwrite(&client->elo_scores, sizeof(int), 1, file);

    len += fwrite_string(client->name, file);
    len += fwrite_string(client->bio, file);
    len += fwrite_string(client->ip, file);
    
    // write Friends Request of client
    for (int i = 0; i < client->nb_friend_req; i++) {
        len += fwrite_string(client->friend_req[i].name_client, file);
        len += fwrite(&client->friend_req[i].status, sizeof(Friend_Req_Status), 1, file);
        len += fwrite(&client->friend_req[i].is_sender, sizeof(bool), 1, file);
    }
    return len;
}

size_t file_read_client(Client *client, FILE *file) {
    // read a client from file

    size_t len = 0;
    size_t t;
    // No need to persist socket and status
    if ((t = fread(&client->nb_friend_req, sizeof(int), 1, file)) != 1 ) {return 0;}
    len += t;
    if ((t = fread(&client->elo_scores, sizeof(int), 1, file)) != 1 ) { return 0;}
    len += t;

    if ((t = fread_string(client->name, file)) == 0) { return 0; }
    len += t;
    if ((t = fread_string(client->bio, file)) == 0) { return 0; }
    len += t;
    if ((t = fread_string(client->ip, file)) == 0) { return 0; }
    len += t;

    // read Friends Request of client
    client->friend_req = (Friend_Req *) malloc(MAX_FRIEND_REQ * sizeof(Friend_Req));
    for (int i = 0; i < client->nb_friend_req; i++) {
        len += fread_string((client->friend_req[i].name_client), file);
        len += fread(&client->friend_req[i].status, sizeof(Friend_Req_Status), 1, file);
        len += fread(&client->friend_req[i].is_sender, sizeof(bool), 1, file);
    }
    return len;
}

void free_client(Client *client) {
    free(client->friend_req);
}