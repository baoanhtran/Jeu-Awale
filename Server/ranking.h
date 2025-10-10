#ifndef RANKING_H
#define RANKING_H

#include "client.h"
#include "game.h"

#define BEGINNER_MAX 1200
#define INTERMEDIATE_MAX 1600
#define ADVANCED_MAX 2000
#define EXPERT_MAX 2400

#define LIMIT_RANK_LIST 50
#define K_COEF 10

double calculate_win_probability(int eloA, int eloB);                             // calculate winning probability, ELO Formule
void updateElo(Client *playerA, Client *playerB, Game_Result result_of_A, int K); // update point elo of player A and player B
int compare_ptr_clients(const void *a, const void *b);                            // need this cause we dont want the sorted list to show affect the origin list
const char *get_rank(int elo);                                                    // get rank by elo score
void display_sorted_players(Client sender, Client players[], int actual);         // print list players in order elo scores in buffer

#endif /* Guard */