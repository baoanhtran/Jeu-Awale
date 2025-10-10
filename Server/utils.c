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
    } // Trim spaces in beginning

    size_t i = 0;
    size_t strSize = min(strlen(str), max_len);

    while (i < strSize - 1 && *str != ' ' && *str != '\0')
    {
        result[i++] = *str++;
    }
    result[i] = '\0'; // Null-terminate

    return str; // Return the pointer to the rest of the string
}

int min(int a, int b)
{
    return a > b ? b : a;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

// Can handle the string "0" (unlike atoi())
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
        str++; // skip whitespace in the beginning
    if (*str == '\0')
        return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--; // skip whitespace at the end
    *(end + 1) = '\0';
}

int strcmp_ip(const char *ip1, const char *ip2)
{
    // check if the two ip are the same
    // cause ip get from cmd line and ip from file are regard to be different although they are the same
    printf("compare ip %s and %s\n", ip1, ip2);

    int len1 = strlen(ip1);
    int len2 = strlen(ip2);
    if (len1 != len2)
    {
        printf("ip1: %s (%lu), ip2: %s (%lu)\n", ip1, strlen(ip1), ip2, strlen(ip2));
        return -1;
    }
    // compare each octet of ip
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