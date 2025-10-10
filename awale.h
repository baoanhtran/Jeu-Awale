#ifndef AWALE_H
#define AWALE_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define BUF_SIZE 2048
#define NB_CASES 12 // Total number of cells
#define GRAINES_PAR_CASE 4 // Number of seeds per cell

typedef struct {
  int plateau[NB_CASES];  // Board
  int score_joueur1;  // Player 1 score
  int score_joueur2;  // Player 2 score
  bool tour_joueur1;  // Is the first player's turn ?
} Partie;

void getPlateau(char* buffer, Partie *partie, int view, const char * name_player_1, const char * name_player_2);
void afficherPlateau(Partie *partie);
Partie* creerPartie();
void detruirePartie(Partie *partie);
bool finDePartie(Partie *partie);
bool verifierCoup(Partie *partie, int case_jouee);
void effectuerTour(Partie *partie, int case_jouee);
void sauvegarderPartie(Partie *partie, const char *nom_fichier);
Partie *chargerPartie(const char *nom_fichier);


#endif
