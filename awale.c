#include <stdio.h>
#include <string.h>
#include "awale.h"

// Copies the board of a partie into a buffer
void getPlateau(char *buffer, Partie *partie, int view, const char *name_player_1, const char *name_player_2)
{
    // print plateau of game to buffer, view = 1 or 2 is for player 1 or 2, the rest number is for observators

    // Temporary buffer to construct the output
    int buffer_size = BUF_SIZE;
    char temp[100];
    buffer[0] = '\0'; // Ensure the buffer starts empty

    // Add "Player 1" at the beginning
    snprintf(temp, sizeof(temp), "\n	  	       %s\n", view == 2 ? "you" : name_player_2);
    strncat(buffer, temp, buffer_size - strlen(buffer) - 1);

    // Print indices for the upper row (Player 2 side)
    for (int i = NB_CASES / 2 - 1; i >= 0; i--)
    {
        snprintf(temp, sizeof(temp), "   (%2d) ", i);
        strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
    }
    strncat(buffer, "\n", buffer_size - strlen(buffer) - 1);

    for (int i = NB_CASES / 2; i < NB_CASES; i++)
    {
        snprintf(temp, sizeof(temp), "--------");
        strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
    }
    strncat(buffer, "--\n", buffer_size - strlen(buffer) - 1);

    // Print the upper row (Player 2 side)
    for (int i = NB_CASES / 2 - 1; i >= 0; i--)
    {
        snprintf(temp, sizeof(temp), "||  %2d  ", partie->plateau[i]);
        strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
    }
    strncat(buffer, "||\n", buffer_size - strlen(buffer) - 1);

    // Print the separator
    for (int i = NB_CASES / 2; i < NB_CASES; i++)
    {
        snprintf(temp, sizeof(temp), "========");
        strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
    }
    strncat(buffer, "==\n", buffer_size - strlen(buffer) - 1);

    // Print the lower row (Player 1 side)
    for (int i = NB_CASES / 2; i < NB_CASES; i++)
    {
        snprintf(temp, sizeof(temp), "||  %2d  ", partie->plateau[i]);
        strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
    }
    strncat(buffer, "||\n", buffer_size - strlen(buffer) - 1);

    for (int i = NB_CASES / 2; i < NB_CASES; i++)
    {
        snprintf(temp, sizeof(temp), "--------");
        strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
    }
    strncat(buffer, "--\n", buffer_size - strlen(buffer) - 1);

    // Print indices for the lower row (Player 1 side)
    for (int i = NB_CASES / 2; i < NB_CASES; i++)
    {
        snprintf(temp, sizeof(temp), "   (%2d) ", i);
        strncat(buffer, temp, buffer_size - strlen(buffer) - 1);
    }
    strncat(buffer, "\n", buffer_size - strlen(buffer) - 1);

    // Add "Player 2" at the end
    snprintf(temp, sizeof(temp), "	  	       %s\n\n", view == 1 ? "you" : name_player_1);
    strncat(buffer, temp, buffer_size - strlen(buffer) - 1);

    // Print scores
    sprintf(temp, "Score of %s: %d\n", view == 1 ? "you" : name_player_1, partie->score_joueur1);
    strncat(buffer, temp, BUF_SIZE - strlen(buffer) - strlen(temp) - 1);
    sprintf(temp, "Score of %s: %d\n", view == 2 ? "you" : name_player_2, partie->score_joueur2);
    strncat(buffer, temp, BUF_SIZE - strlen(buffer) - strlen(temp) - 1);
    strncat(buffer, "=========================\n\n", BUF_SIZE - strlen(buffer) - 1);
}

// Print the board of a partie in stdout (test purpose)
void afficherPlateau(Partie *partie)
{
    printf("\n=========================");
    printf("\n        Joueur 2\n");
    for (int i = NB_CASES / 2 - 1; i >= 0; i--)
    {
        printf(" %2d ", partie->plateau[i]);
    }
    printf("\n");
    for (int i = NB_CASES / 2; i < NB_CASES; i++)
    {
        printf(" %2d ", partie->plateau[i]);
    }
    printf("\n        Joueur 1\n\n");

    printf("Score Joueur 1: %d\n", partie->score_joueur1);
    printf("Score Joueur 2: %d\n", partie->score_joueur2);
    printf("=========================\n\n");
}

// Creates and initialize a Partie
Partie *creerPartie()
{
    Partie *nouvelle_partie = (Partie *)malloc(sizeof(Partie));
    if (!nouvelle_partie)
    {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    // Initialisation du plateau avec 4 graines dans chaque case
    for (int i = 0; i < NB_CASES; i++)
    {
        nouvelle_partie->plateau[i] = GRAINES_PAR_CASE;
    }

    nouvelle_partie->score_joueur1 = 0;
    nouvelle_partie->score_joueur2 = 0;

    nouvelle_partie->tour_joueur1 = rand() % 2; // Random starting player

    return nouvelle_partie;
}

bool finDePartie(Partie *partie)
{
    int graines_total = 0;
    int graines_joueur1 = 0;
    int graines_joueur2 = 0;

    // Calcul du total des graines et des graines pour chaque joueur
    for (int i = 0; i < NB_CASES; i++)
    {
        graines_total += partie->plateau[i];
        if (i < 6)
        {
            graines_joueur2 += partie->plateau[i];
        }
        else
        {
            graines_joueur1 += partie->plateau[i];
        }
    }

    // Cas 1 : Le plateau est trop peu pour jouer
    if (graines_total <= 3)
    {
        return true;
    }

    // Cas 2 : Un joueur n’a plus de graines dans ses cases
    if ((partie->tour_joueur1 && graines_joueur1 == 0) || (!partie->tour_joueur1 && graines_joueur2 == 0))
    {
        return true;
    }

    return false;
}

// Fonction pour vérifier la validité d'un coup
bool verifierCoup(Partie *partie, int case_jouee)
{
    // Vérifie que la case est dans les limites du joueur actif
    // joeur 1: 6-11
    // joeur 2: 0-5
    if ((partie->tour_joueur1 && (case_jouee < 6 || case_jouee > 11)) ||
        (!partie->tour_joueur1 && (case_jouee < 0 || case_jouee > 5)))
    {
        return false;
    }

    // Vérifie que la case contient des graines
    if (partie->plateau[case_jouee] == 0)
    {
        return false;
    }

    // Simule le coup pour vérifier qu'il ne génère pas de famine
    int plateau_simule[NB_CASES];
    for (int i = 0; i < NB_CASES; i++)
    {
        plateau_simule[i] = partie->plateau[i];
    }

    int graines = plateau_simule[case_jouee];
    plateau_simule[case_jouee] = 0;

    int position = case_jouee;
    while (graines > 0)
    {
        position = (position + 1) % NB_CASES;
        plateau_simule[position]++;
        graines--;
    }

    // Vérifie si l'adversaire a des graines dans ses cases après le coup simulé
    int debut_adversaire = partie->tour_joueur1 ? 0 : 6;
    int fin_adversaire = partie->tour_joueur1 ? 5 : 11;
    bool adversaire_a_graines = false;

    for (int i = debut_adversaire; i <= fin_adversaire; i++)
    {
        if (plateau_simule[i] > 0)
        {
            adversaire_a_graines = true;
            break;
        }
    }

    // Si l'adversaire n'a plus de graines, cela génère une famine et le coup est
    // invalide
    return adversaire_a_graines;
}

bool verifierPrises(Partie *partie, int position)
{
    // Vérifie si c'est une case du camp adverse.
    if ((partie->tour_joueur1 && !(position < 6 || position > 11)) ||
        (!partie->tour_joueur1 && !(position < 0 || position > 5)))
    {
        return false;
    }

    // Vérifie si contient 2 ou 3 graines
    if (partie->plateau[position] <= 2 || partie->plateau[position] >= 3)
    {
        return false;
    }
    return true;
}

// Plays a turn (test purpose)
void effectuerTour(Partie *partie, int case_jouee)
{
    // Récupérer le nombre de graines de la case jouée
    int graines = partie->plateau[case_jouee];
    partie->plateau[case_jouee] = 0; // La case jouée devient vide

    int position = case_jouee;

    // Distribuer les graines
    while (graines > 0)
    {
        position =
            (position + 1) % NB_CASES; // Passer à la case suivante, en boucle
        partie->plateau[position]++;
        graines--;
    }

    // Changer de joueur
    partie->tour_joueur1 = !partie->tour_joueur1;
}

// Saves a game
void sauvegarderPartie(Partie *partie, const char *nom_fichier)
{
    FILE *fichier = fopen(nom_fichier, "w");
    if (!fichier)
    {
        fprintf(stderr, "Erreur d'ouverture du fichier de sauvegarde.\n");
        return;
    }

    // Sauvegarder l'état du plateau (12 cases)
    for (int i = 0; i < NB_CASES; i++)
    {
        fprintf(fichier, "%d ", partie->plateau[i]);
    }
    fprintf(fichier, "\n");

    // Sauvegarder les scores des joueurs
    fprintf(fichier, "%d %d\n", partie->score_joueur1, partie->score_joueur2);

    // Sauvegarder le joueur actuel (1 ou 2)
    fprintf(fichier, "%d\n", partie->tour_joueur1 ? 1 : 2);

    fclose(fichier);
    printf("Partie sauvegardée avec succès dans '%s'.\n", nom_fichier);
}

// Loads a game
Partie *chargerPartie(const char *nom_fichier)
{
    FILE *fichier = fopen(nom_fichier, "r");
    if (!fichier)
    {
        fprintf(stderr, "Erreur d'ouverture du fichier de sauvegarde.\n");
        return NULL;
    }

    Partie *partie_chargee = (Partie *)malloc(sizeof(Partie));
    if (!partie_chargee)
    {
        fprintf(stderr, "Erreur d'allocation mémoire pour la partie.\n");
        fclose(fichier);
        return NULL;
    }

    // Charger l'état du plateau (12 cases)
    for (int i = 0; i < NB_CASES; i++)
    {
        if (fscanf(fichier, "%d", &partie_chargee->plateau[i]) != 1)
        {
            fprintf(stderr, "Erreur de lecture du plateau depuis le fichier.\n");
            free(partie_chargee);
            fclose(fichier);
            return NULL;
        }
    }

    // Charger les scores des joueurs
    if (fscanf(fichier, "%d %d", &partie_chargee->score_joueur1, &partie_chargee->score_joueur2) != 2)
    {
        fprintf(stderr, "Erreur de lecture des scores depuis le fichier.\n");
        free(partie_chargee);
        fclose(fichier);
        return NULL;
    }

    // Charger le joueur actuel (1 ou 2)
    int tour_joueur;
    if (fscanf(fichier, "%d", &tour_joueur) != 1)
    {
        fprintf(stderr, "Erreur de lecture du tour du joueur depuis le fichier.\n");
        free(partie_chargee);
        fclose(fichier);
        return NULL;
    }
    partie_chargee->tour_joueur1 = (tour_joueur == 1);

    fclose(fichier);
    printf("Partie chargée avec succès depuis '%s'.\n", nom_fichier);

    return partie_chargee;
}

// Frees a Partie
void detruirePartie(Partie *partie) { free(partie); }
