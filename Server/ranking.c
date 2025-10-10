#include <stdio.h>
#include <math.h>
#include <string.h>

#include "client.h"
#include "game.h"
#include "ranking.h"
#include "utils.h"
#include "server.h"

double calculate_win_probability(int eloA, int eloB) {  //ELO Formule
    return 1.0 / (1.0 + pow(10.0, (eloB - eloA) / 400.0));  
} 

void updateElo(Client *playerA, Client *playerB, Game_Result result_of_A, int K) {  
    //update point elo of player A and player B

    double PA = calculate_win_probability(playerA->elo_scores, playerB->elo_scores);  
    double PB = calculate_win_probability(playerB->elo_scores, playerA->elo_scores);  

    int resultA = 0;
    switch (result_of_A) {
        case WIN:
            resultA = 1;
            break;
        case DRAW:
            resultA = 0;
            break;
        default:
            break;
    }
    
    // Cập nhật điểm  
    playerA->elo_scores += K * (resultA - PA);  
    playerB->elo_scores += K * ((1 - resultA) - PB);  
} 

int compare_ptr_clients(const void *a, const void *b) {  
    //need this cause we dont want the sorted list to show affect the origin list

    Client* ptr_A = *(Client**)a;  
    Client* ptr_B = *(Client**)b;  
    
    return ptr_B->elo_scores - ptr_A->elo_scores; //In decrease
}  

const char* get_rank(int elo) {  
    //get rank by elo score

    if (elo < BEGINNER_MAX) {  
        return "Beginner";  
    } else if (elo < INTERMEDIATE_MAX) {  
        return "Intermediate";  
    } else if (elo < ADVANCED_MAX) {  
        return "Advanced";  
    } else if (elo < EXPERT_MAX) {  
        return "Expert";  
    } else {  
        return "Master";  
    }  
} 

void display_sorted_players(Client sender, Client* clients, int actual) {
    //print list players in order elo scores in buffer

    char buffer[BUF_SIZE];

    Client **ptr_clients = malloc( actual * sizeof(Client*) ); //we use list ptr_clients, order this instead, so it do not affect the origin list clients
    
    //create array of indeces 
    for (int i = 0; i < actual; i++) {  
        ptr_clients[i] = &clients[i];
    }  

    //sort the indices array base on client elo scores
    qsort(ptr_clients, actual, sizeof(Client*), compare_ptr_clients);  
    write_client(sender.sock, "123");
    // show list 
    sprintf(buffer, "Sorted rankings of all clients:\n");  
    for (int i = 0; i < min(actual, LIMIT_RANK_LIST); i++) {  
        Client *ptr = ptr_clients[i];

        const char* rank = get_rank(ptr->elo_scores);

        char info[BUF_SIZE];
        sprintf(info, 
            "%d. [%s] %s - score: %d\n",
            i+1, rank, ptr->name, ptr->elo_scores
        );  
        strcat(buffer, info);
    }  

    write_client(sender.sock, buffer);
}  
