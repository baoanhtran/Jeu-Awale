#ifndef USER_INTERACTION_H
#define USER_INTERACTION_H

#include <stdbool.h>
#include "client.h"

void chat(const Client *clients, int actual, Client sender, const char *receiver_msg); // envoyer le message de sender au destinataire
void set_bio(Client *ptr_clients, const char *bio);                                    // définir la bio du client
void show_bio(const Client *clients, int actual, Client sender, const char *name);     // envoyer la bio de name au sender

// amis
int make_friend(Client *clients, int actual, int ind_sender, const char *name_requested);                        // envoyer une demande d'ami
void pending_friend_request(Client *ptr_sender, int ind_fr_sender, Client *ptr_requested, int ind_fr_requested); // mettre à jour les demandes d'ami du sender et du requested aux index correspondants
int accept_friend_req(Client acceptor, const char *name, Client clients[], int actual);                          // accepter une demande d'ami
int deny_friend_request(Client refuser, const char *name, Client clients[], int actual);                         // refuser une demande d'ami
int cancel_friend_request(Client refuser, const char *name, Client clients[], int actual);                       // annuler une demande d'ami
int unfriend(Client demander, const char *name, Client clients[], int actual);                                   // retirer l'ami entre demander et name
int is_friend(Client client, const char *name);                                                                  // vérifie si name est ami du client, retourne l'index sinon -1
int find_fr_req(Client client, const char *name_to_find);                                                        // trouve dans les FR du client si une FR existe pour name_to_find

#endif /* guard */