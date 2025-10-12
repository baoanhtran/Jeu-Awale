#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "client.h"
#include "utils.h"

const char *extract_first_word(const char *str, char *result, int max_len)
{
    while (*str == ' ')
    {
        str++;
    } // Supprime les espaces en début

    size_t i = 0;
    size_t strSize = min(strlen(str), max_len);

    while (i < strSize - 1 && *str != ' ' && *str != '\0')
    {
        result[i++] = *str++;
    }
    result[i] = '\0'; // Termine par '\0'

    return str; // Retourne le pointeur vers le reste de la chaîne
}

int min(int a, int b)
{
    return a > b ? b : a;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

// Peut gérer la chaîne "0" (contrairement à atoi())
int my_atoi(const char *cell_str, int *cell)
{
    char *fin;
    errno = 0;
    long valeur = strtol(cell_str, &fin, 10);
    if (fin == cell_str)
    {
        return -1;
    }
    if (*fin != '\0')
    {
        return -1;
    }
    if ((valeur == LONG_MIN || valeur == LONG_MAX) && errno == ERANGE)
    {
        return -1;
    }
    if (valeur < INT_MIN || valeur > INT_MAX)
    {
        return 0;
    }
    *cell = (int)valeur;
    return 0;
}

void trim_whitespace(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
        str++; // ignore les espaces en début
    if (*str == '\0')
        return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--; // ignore les espaces en fin
    *(end + 1) = '\0';
}

int strcmp_ip(const char *ip1, const char *ip2)
{
    // vérifie si les deux IP sont identiques
    // car une IP récupérée depuis la ligne de commande et depuis le fichier peuvent sembler différentes
    printf("comparaison des IP %s et %s\n", ip1, ip2);

    int len1 = strlen(ip1);
    int len2 = strlen(ip2);
    if (len1 != len2)
    {
        printf("ip1: %s (%lu), ip2: %s (%lu)\n", ip1, strlen(ip1), ip2, strlen(ip2));
        return -1;
    }
    // compare chaque octet de l'IP
    for (int i = 0; i <= len1; i++)
    {
        if (ip1[i] != ip2[i])
        {
            printf("ip1: %s, ip2: %s\n ( %c != %c )", ip1, ip2, ip1[i], ip2[i]);
            return ip1[i] - ip2[i];
        }
    }
    return 0;

    return strncmp(ip1, ip2, len1);
}