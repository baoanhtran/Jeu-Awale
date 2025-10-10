#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#define FILENAME "./data/clients.dat"

#include <stdbool.h>
#include "client.h"

// Function declarations
void addClient(Client client);                              // add a client to database
void getClients(Client *clients, int *nb_clients);          // read all clients from database
void updateClient(Client newClient);                        // update a client in database by name
void deleteClient(const char *name);                        // delete a client from database by name
size_t fwrite_string(const char *str, FILE *file);          // write string to file
size_t fread_string(char *ptr_buffer, FILE *file);          // read string from file (string has been added by fwrite_string)
size_t file_write_client(const Client *client, FILE *file); // write a client to file
size_t file_read_client(Client *client, FILE *file);        // read a client from file
void free_client(Client *client);

#endif // PERSISTENCE_H