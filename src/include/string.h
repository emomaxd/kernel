#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

size_t strlen(const char *str);

char *strcpy(char *dest, const char *src);

char *strncpy(char *dest, const char *src, size_t n);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t n);

char *strcat(char *dest, const char *src);

char *strncat(char *dest, const char *src, size_t n);

char *strchr(const char *str, int c);

char *strrchr(const char *str, int c);

char *strstr(const char *haystack, const char *needle);

char *strtok(char *str, const char *delim);

#endif
