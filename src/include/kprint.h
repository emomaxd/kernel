#ifndef KPRINT_H
#define KPRINT_H

#include <stddef.h>
#include <stdint.h>

void itoa(int num, char *str, int base);
int atoi(const char *str);


void kprint(const char *str, size_t x, size_t y, uint32_t color);
void kprint_hex(uint32_t value, size_t x, size_t y, uint32_t color);
void kprint_dec(uint32_t value, size_t x, size_t y, uint32_t color);

#endif // KPRINT_H
