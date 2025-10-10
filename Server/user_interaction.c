#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "client.h"
#include "utils.h"
#include "server.h"
#include "message.h"
#include "user_interaction.h"

void chat(const Client *clients, int actual, Client sender, const char *receiver_msg) {	
	//send a chat from sender to receiver

	//check params of cmd
	if (*receiver_msg == '\0') {
		fwrite_client(sender.sock, "Name & Message not found! You need to specify the msg follow the syntax: chat [pseudo] [msg]");
		return;
	} 

	//extract name receiver and msg in command
	char receiver[NAME_SIZE];
	const char *msg = extract_first_word(receiver_msg, receiver, NAME_SIZE) + 1;

	if (*msg == '\0') {
		fwrite_client(sender.sock, "Msg not found! You need to specify the msg follow the syntax: chat [pseudo] [msg]");
		return;
	} 

	//check if name exist
	int receiver_client_idx = name_exists(clients, actual, receiver);
	if (receiver_client_idx == -1) {
		fwrite_client(
			sender.sock,
			"This pseudo doesn't exist"
		);
		return;
	}

	//check status of client
	if (clients[receiver_client_idx].status == OFFLINE) {
		fwrite_client(
			sender.sock,
			"%s is offline! He is not able to get the chat message",
			clients[receiver_client_idx].name
		);
		return;
	}

	fwrite_client(
		clients[receiver_client_idx].sock, 
		"%s: %s",
		sender.name, msg
	);
}

void set_bio(Client *ptr_client, const char * bio) {
	//set bio of client
	strncpy(ptr_client->bio, bio, BIO_SIZE - 1);
	ptr_client->bio[BIO_SIZE - 1] = '\0';
}

void show_bio(const Client *clients, int actual, Client sender, const char *name) {
	//send bio of name to sender

	//check params of cmd
	if (*name == '\0') {
		fwrite_client(
			sender.sock, 
			"Name not found! You need to follow the syntax: bio [pseudo]"
		);
		return;
	}

	//check if name exist
	int client_idx = name_exists(clients, actual, name);
	if (client_idx != -1) {
		fwrite_client(
			sender.sock,
			"bio of %s: %s",
			clients[client_idx].name, 
			*clients[client_idx].bio == '\0'
				? "No bio added yet!"
				: clients[client_idx].bio
		);
	} else {
		fwrite_client(
			sender.sock,
			"This pseudo doesn't exist"
		);
	}

}


int make_friend(Client *clients, int actual, int ind_sender, const char *name_requested) {
	//send friend request

	//check params of cmd
	if (*name_requested == '\0') {
		fwrite_client(clients[ind_sender].sock, 
			"Name not found! You need to follow the syntax: make friend [pseudo]"
		);
		return -1;
	}
	
	//check if make friend with himself
	if ( strcmp( clients[ind_sender].name, name_requested) == 0 ) {
		fwrite_client(clients[ind_sender].sock, "You can not send friend request to yourself!" );
		return -1;
	}

	//check if player exist!
	int client_requested_id = name_exists(clients, actual, name_requested);
	if (client_requested_id == -1) {
		fwrite_client(clients[ind_sender].sock, "Player %s doesn't exist!", name_requested );
		return -1;
	}

	//check if already friends
	Client sender = clients[ind_sender];
	Client requested = clients[client_requested_id];
	int ind_fr_sender = find_fr_req(sender, name_requested);

	//case existed
	if (ind_fr_sender != -1) {
		Friend_Req fr = sender.friend_req[ind_fr_sender];
		switch (fr.status){
			case ACCEPTED:
				fwrite_client(sender.sock, "You and %s are already friend!", name_requested );
				return -1;
			case PENDING:
				fwrite_client(sender.sock, "A friend request between you exist, please accept or deny!", name_requested );
				return -1;
			case DENIED:
				fwrite_client(sender.sock, "A request between you has denied one time! Another request will be sent!" );
				break;
			default:
				break;
		}
	}

	//create friends requests if not exist
	if (requested.friend_req == NULL) {clients[client_requested_id].friend_req = malloc( MAX_FRIEND_REQ * sizeof(Friend_Req) );}
	if (sender.friend_req == NULL) {clients[ind_sender].friend_req = malloc( MAX_FRIEND_REQ * sizeof(Friend_Req) );}

	//create friend requests and send message to them
	int ind_fr_requested = find_fr_req(requested, sender.name);
	pending_friend_request(&clients[ind_sender], ind_fr_sender, &clients[client_requested_id], ind_fr_requested);
	send_msg_friend_request( sender, requested );

	return client_requested_id;
}

void pending_friend_request( Client *ptr_sender, int ind_fr_sender, Client *ptr_requested, int ind_fr_requested ) {
	//update friend request of sender and requested at the index of friends requests correspondant
	//case ind_fr = -1 means create new friend req

	int id_fr_sender = ind_fr_sender == -1
						? ptr_sender->nb_friend_req
						: ind_fr_sender;
	int id_fr_requested = ind_fr_requested == -1
							? ptr_requested->nb_friend_req
							: ind_fr_requested;

	//update friend request for sender
	strcpy( (ptr_sender->friend_req[  id_fr_sender ]).name_client, ptr_requested->name);
	(ptr_sender->friend_req[  id_fr_sender ]).status = PENDING;
	(ptr_sender->friend_req[  id_fr_sender ]).is_sender = false;

	//Update friend request for client requested
	strcpy( (ptr_requested->friend_req[  id_fr_requested ]).name_client, ptr_sender->name );
	(ptr_requested->friend_req[  id_fr_requested ]).status = PENDING;
	(ptr_requested->friend_req[  id_fr_requested ]).is_sender = true;

	//case create new friend request, nb_fr need to update
	if (ind_fr_sender == -1) { ptr_sender->nb_friend_req++; }
	if (ind_fr_requested == -1) { ptr_requested->nb_friend_req++; }
}

int accept_friend_req(Client acceptor, const char *name, Client clients[], int actual) {
	//accept a friend request

	//check params of cmd
	if (*name == '\0') {
		fwrite_client(acceptor.sock, 
			"Name not found! You need to follow the syntax: accept fr [pseudo]"
		);
		return -1;
	}

	//check if exist fr from name
	int ind_fr_acceptor = find_fr_req(acceptor, name);

	//case not exist
	if (ind_fr_acceptor == -1) {
		fwrite_client( acceptor.sock, "You dont have friend request from %s!", name );
		return -1;
	}
	Friend_Req fr = acceptor.friend_req[ ind_fr_acceptor ];

	//case acceptor is the sender of friend req, so can not accept
	if (!fr.is_sender && fr.status != DELETED) {
		fwrite_client( acceptor.sock, "You are the one who sent friend request, so you can not accept! Cancel it if you want!" );
		return -1;
	}

	//handle different cases of status
	switch (fr.status) {
		case ACCEPTED:
			fwrite_client(acceptor.sock, "You have already accepted friend request!");
			return -1;
		case DENIED:
			fwrite_client(acceptor.sock, "You have already denied friend request. Please send another friend request with 'make friend' command!");
			return -1;
		case DELETED:
			fwrite_client( acceptor.sock, "You dont have friend request from %s!", name );
			return -1;
		default:
			break;
	}

	//case exist: update 2 friend request both 2 sides
	acceptor.friend_req[ ind_fr_acceptor ].status = ACCEPTED;
	int ind_sender = name_exists( clients, actual, acceptor.friend_req[ind_fr_acceptor].name_client);
	int ind_fr_sender = find_fr_req( clients[ind_sender], acceptor.name );
	clients[ind_sender].friend_req[ ind_fr_sender ].status = ACCEPTED;

	fwrite_client(acceptor.sock, "You and %s are now friend!", clients[ind_sender].name );
	if (clients[ind_sender].status != OFFLINE) {
		fwrite_client(clients[ind_sender].sock, "%s accepted your friend request!", acceptor.name );
	}
	return ind_sender;
}

int deny_friend_request(Client refuser, const char *name, Client clients[], int actual) {
	//deny a friend request

	//check params of cmd
	if (*name == '\0') {
		fwrite_client(refuser.sock, 
			"Name not found! You need to follow the syntax: deny fr [pseudo]"
		);
		return -1;
	}

	//check if exist fr from name
	int ind_fr_refuser = find_fr_req(refuser, name);

	//case not exist
	if (ind_fr_refuser == -1) {
		fwrite_client( refuser.sock, "You dont have friend request from %s!", name );
		return -1;
	}
	Friend_Req fr = refuser.friend_req[ ind_fr_refuser ];

	//case refuser is the sender of friend req, so can not deny
	if (!fr.is_sender && fr.status != DELETED) {
		fwrite_client( refuser.sock, "You are the one who sent friend request, so you can not deny! Cancel it instead!" );
		return -1;
	}

	//handle different cases of status
	switch (fr.status) {
		case ACCEPTED:
			fwrite_client(refuser.sock, "You have already accepted friend request! Please unfriend instead!");
			return -1;
		case DENIED:
			fwrite_client(refuser.sock, "You have already denied friend request. No need to denied more @@");
			return -1;
		case DELETED:
			fwrite_client(refuser.sock, "You dont have friend request from %s!", name );
			return -1;
		default:
			break;
	}

	//case exist: update 2 friend request both 2 sides
	refuser.friend_req[ ind_fr_refuser ].status = DENIED;
	int ind_sender = name_exists( clients, actual, refuser.friend_req[ind_fr_refuser].name_client);
	int ind_fr_sender = find_fr_req( clients[ind_sender], refuser.name );
	clients[ind_sender].friend_req[ ind_fr_sender ].status = DENIED;

	fwrite_client(refuser.sock, "Deny friend request successfully!", clients[ind_sender].name );
	if (clients[ind_sender].status != OFFLINE) {
		fwrite_client(clients[ind_sender].sock, "%s denied your friend request!", refuser.name );
	}
	
	return ind_sender;
}

int cancel_friend_request(Client refuser, const char *name, Client clients[], int actual) {
	//deny a friend request

	//check params of cmd
	if (*name == '\0') {
		fwrite_client(refuser.sock, 
			"Name not found! You need to follow the syntax: cancel fr [pseudo]"
		);
		return -1;
	}

	//check if exist fr from name
	int ind_fr_refuser = find_fr_req(refuser, name);

	//case not exist
	if (ind_fr_refuser == -1) {
		fwrite_client( refuser.sock, "You have not sent friend request to %s!", name );
		return -1;
	}
	Friend_Req fr = refuser.friend_req[ ind_fr_refuser ];

	//case refuser is the sender of friend req, so can not deny
	if (fr.is_sender  && fr.status != DELETED) {
		fwrite_client( refuser.sock, "You are not the one who sent friend request, so you can not cancel it! Deny it instead!" );
		return -1;
	}

	//handle different cases of status
	switch (fr.status) {
		case ACCEPTED:
			fwrite_client(refuser.sock, "%s have already accepted friend request! Please unfriend instead!", name);
			return -1;
		case DENIED:
			fwrite_client(refuser.sock, "%s have already denied friend request.", name);
			return -1;
		case DELETED:
			fwrite_client(refuser.sock, "You dont have friend request from %s!", name );
			return -1;
		default:
			break;
	}

	//case exist: update 2 friend request both 2 sides
	refuser.friend_req[ ind_fr_refuser ].status = DELETED;
	int ind_sender = name_exists( clients, actual, refuser.friend_req[ind_fr_refuser].name_client);
	int ind_fr_sender = find_fr_req( clients[ind_sender], refuser.name );
	clients[ind_sender].friend_req[ ind_fr_sender ].status = DELETED;

	fwrite_client( refuser.sock, "You cancel your friend request to %s!", name );
	return ind_sender;
}

int unfriend( Client demander, const char * name, Client clients[], int actual) {
	//unfriend

	//check params of cmd
	if (*name == '\0') {
		fwrite_client(demander.sock, 
			"Name not found! You need to follow the syntax: unfriend [pseudo]"
		);
		return -1;
	}

	//check if exist fr from name
	int ind_fr_demander = find_fr_req(demander, name);

	//case not exist
	if (ind_fr_demander == -1) {
		fwrite_client( demander.sock, "You have not sent friend request to %s!", name );
		return -1;
	}
	Friend_Req fr = demander.friend_req[ ind_fr_demander ];

	//if friend request already deleted
	if (fr.status == DELETED) {
		fwrite_client( demander.sock, "You and %s are not friend, don't worry!", name);
		return -1;
	}

	//handle different cases of status
	switch (fr.status) {
		case DENIED:
			fwrite_client(demander.sock, "%s have already accepted friend request before!", fr.is_sender ? "You" : name);
			return -1;
		case PENDING:
			fwrite_client(demander.sock, "There are friend request between you! Use %s command instead!", fr.is_sender ? "deny fr" : "cancel fr");
			return -1;
		default:
			break;
	}

	//case exist: update 2 friend request both 2 sides
	demander.friend_req[ ind_fr_demander ].status = DELETED;
	int ind_sender = name_exists( clients, actual, demander.friend_req[ind_fr_demander].name_client);
	int ind_fr_sender = find_fr_req( clients[ind_sender], demander.name );
	clients[ind_sender].friend_req[ ind_fr_sender ].status = DELETED;

	fwrite_client( demander.sock, "You and %s are no longer friend!", name );
	return ind_sender;
}

int find_fr_req(Client client, const char *name_to_find) {
	//find in FR of client if have FR of name_to_find

	for (int i = 0; i < client.nb_friend_req; i++) {
		if ( strcmp(name_to_find, (client.friend_req[i]).name_client) == 0 ) {
			return i;
		}
	}
	return -1;
}

int is_friend(Client client, const char *name) {
	int ind_fr_req = find_fr_req(client, name);
	if (ind_fr_req == -1 || client.friend_req[ind_fr_req].status == ACCEPTED) {
		return ind_fr_req;
	}
	return -1;
}