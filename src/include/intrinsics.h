#ifndef INTRINSICS_H
#define INTRINSICS_H

#include <stdint.h>
#include <stddef.h>

// Bitwise operations
uint32_t _rotl(uint32_t value, uint32_t shift);
uint32_t _rotr(uint32_t value, uint32_t shift);
uint32_t _bswap(uint32_t value);
uint32_t _clz(uint32_t value);  // Count leading zeros
uint32_t _popcnt(uint32_t value);  // Count number of set bits

// I/O port access
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);

void outl(uint16_t port, uint32_t value);
uint32_t inl(uint16_t port);

void acpi_shutdown(void);
void hcf(void);

// Memory operations
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);


#endif // INTRINSICS_H

