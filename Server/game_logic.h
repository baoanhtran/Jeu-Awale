#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define BUF_SIZE 2048
#define NB_CASES 12        // Nombre total de cases
#define GRAINES_PAR_CASE 4 // Nombre de graines par case

typedef struct
{
    int plateau[NB_CASES]; // Plateau
    int score_joueur1;     // Score du joueur 1
    int score_joueur2;     // Score du joueur 2
    bool tour_joueur1;     // Est-ce au tour du joueur 1 ?
} Partie;

void getPlateau(char *buffer, Partie *partie, int view, const char *name_player_1, const char *name_player_2);
void afficherPlateau(Partie *partie);
Partie *creerPartie();
bool finDePartie(Partie *partie);
bool verifierCoup(Partie *partie, int case_jouee);
void effectuerTour(Partie *partie, int case_jouee);
void sauvegarderPartie(Partie *partie, const char *nom_fichier);
Partie *chargerPartie(const char *nom_fichier);

#endif
