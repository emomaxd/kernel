#include "intrinsics.h"
#include <stddef.h>
#include <stdint.h>

// Bitwise operations
uint32_t _rotl(uint32_t value, uint32_t shift) {
    return (value << shift) | (value >> (32 - shift));
}

uint32_t _rotr(uint32_t value, uint32_t shift) {
    return (value >> shift) | (value << (32 - shift));
}

uint32_t _bswap(uint32_t value) {
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0xFF000000) >> 24);
}

uint32_t _clz(uint32_t value) {
    uint32_t count = 0;
    if (value == 0) return 32;  // Edge case: count leading zeros in 0 is 32

    while ((value & 0x80000000) == 0) {  // Check the most significant bit
        count++;
        value <<= 1;  // Shift left by 1 bit
    }
    return count;
}

uint32_t _popcnt(uint32_t value) {
    uint32_t count = 0;
    while (value) {
        count += value & 1;  // Increment count if the least significant bit is 1
        value >>= 1;  // Shift right by 1 bit
    }
    return count;
}

void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

// I/O port access
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" :: "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" :: "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" :: "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Basic memory functions
void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// ACPI shutdown function
inline void acpi_shutdown(void) {
    outw(0xB004, 0x2000);  // Common ACPI shutdown port

    // Alternative ports if the first doesn't work
    outw(0x604, 0x2000);   // Common for QEMU
    outw(0x4004, 0x3400);  // Another common port
}