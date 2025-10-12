#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#define FILENAME "./data/clients.dat"

#include <stdbool.h>
#include "client.h"

// Déclarations des fonctions
void addClient(Client client);                              // ajoute un client à la base
void getClients(Client *clients, int *nb_clients);          // lit tous les clients depuis la base
void updateClient(Client newClient);                        // met à jour un client dans la base par nom
void deleteClient(const char *name);                        // supprime un client de la base par nom
size_t fwrite_string(const char *str, FILE *file);          // écrit une chaîne dans le fichier
size_t fread_string(char *ptr_buffer, FILE *file);          // lit une chaîne depuis le fichier (écrite par fwrite_string)
size_t file_write_client(const Client *client, FILE *file); // écrit un client dans le fichier
size_t file_read_client(Client *client, FILE *file);        // lit un client depuis le fichier
void free_client(Client *client);

#endif // PERSISTENCE_H