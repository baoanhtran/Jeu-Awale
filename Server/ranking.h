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

double calculate_win_probability(int eloA, int eloB);                             // calcule la probabilité de victoire, formule ELO
void updateElo(Client *playerA, Client *playerB, Game_Result result_of_A, int K); // met à jour les points ELO des joueurs A et B
int compare_ptr_clients(const void *a, const void *b);                            // nécessaire pour ne pas modifier l'ordre original lors du tri
const char *get_rank(int elo);                                                    // obtient le rang d'après le score ELO
void display_sorted_players(Client sender, Client players[], int actual);         // affiche la liste triée des joueurs par ELO

#endif /* Guard */