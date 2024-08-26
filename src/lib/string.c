#include "string.h"

size_t strlen(const char *str)
{
	const char *s = str;
	while (*s)
		++s;
	return s - str;
}

char *strcopy(char *dest, const char *src)
{
	char *d = dest;
	while ((*d++ = *src++));
	return dest;
}

int strcmp(const char *str1, const char *str2)
{
	while(*str1 && (*str1 == *str2))
	{
		str1++;
		str2++;
	}
	return *(unsigned char *)str1 - *(unsigned char *)str2;
}

int strncmp(const char *str1, const char *str2, size_t n)
{
    while (n--)
    {
        if (*str1 != *str2 || *str1 == '\0')
	{
            return (unsigned char)*str1 - (unsigned char)*str2;
        }
        str1++;
        str2++;
    }
    return 0;
}

char *strcat(char *dest, const char *src)
{
	char *d = dest;
	while (*d) d++;
	while ((*d++ = *src++));
	return dest;
}

char *strchr(const char *str, int c)
{
	while(*str)
	{
		if (*str == (char)c) return (char *)str;
		str++;
	}
	return (c == '\0') ? (char *)str : NULL;
}

char *strrchr(const char *str, int c)
{
	const char *last = NULL;
	do 
	{
		if (*str == (char)c) last = str;
	} while (*str++);
	return (char *)last;
}

char *strstr(const char *haystack, const char *needle)
{
	if (!*needle) return (char *)haystack;

	for (; *haystack; haystack++)
	{
		const char *h = haystack;
		const char *n =  needle;
		while (*h && *n && *h == *n)
		{
			++h;
			++n;
		}
		if (!*n) return (char *)haystack; // haystack includes the full needle
	}
	return NULL;
}

char *strtok(char *str, const char *delim)
{
	static char *next;
	if (str) next = str;

	if (!next) return NULL;

	str = next;
	while (*str && strchr(delim, *str)) str++;
	
	if (!*str)
	{
		next = NULL;
		return NULL;
	}

	next = str;
	while (*next && !strchr(delim, *next)) next++;

	if (*next) *next++ = '\0';

	return str;
}
