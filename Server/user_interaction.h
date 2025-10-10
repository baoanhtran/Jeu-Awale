#ifndef USER_INTERACTION_H
#define USER_INTERACTION_H

#include <stdbool.h>
#include "client.h"

void chat(const Client *clients, int actual, Client sender, const char * receiver_msg); //send message of sender to receiver
void set_bio(Client *ptr_clients, const char * bio); //set bio of client
void show_bio(const Client *clients, int actual, Client sender, const char *name); //send bio of name to sender

//friends
int make_friend(Client *clients, int actual, int ind_sender, const char *name_requested); //send friend request
void pending_friend_request( Client *ptr_sender, int ind_fr_sender, Client *ptr_requested, int ind_fr_requested ); //update friend request of sender and requested at the index of friends requests correspondant
int accept_friend_req(Client acceptor, const char *name, Client clients[], int actual); //accept a friend request
int deny_friend_request(Client refuser, const char *name, Client clients[], int actual); //deny a friend request
int cancel_friend_request(Client refuser, const char *name, Client clients[], int actual); //cancel a friend request
int unfriend( Client demander, const char * name, Client clients[], int actual); //unfriend between demander and name
int is_friend(Client client, const char *name); //check if name is friend of client, return index of friend req correspondant, if not return -1
int find_fr_req(Client client, const char *name_to_find); //find in FR of client if have FR of name_to_find


#endif /* guard */