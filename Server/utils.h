#ifndef UTILS_H
#define UTILS_H

const char *extract_first_word(const char *str, char *result, int max_len);
int min(int a, int b);
int max(int a, int b);
int my_atoi(const char *cell_str, int *cell);
void trim_whitespace(char *str);
int strcmp_ip(const char *ip1, const char *ip2);

#endif