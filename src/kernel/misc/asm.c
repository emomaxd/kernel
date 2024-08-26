#include "asm.h"

// Halt and catch fire function
void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

// Port I/O functions
inline void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// ACPI shutdown function
inline void acpi_shutdown(void) {
    outw(0xB004, 0x2000);  // Common ACPI shutdown port

    // Alternative ports if the first doesn't work
    outw(0x604, 0x2000);   // Common for QEMU
    outw(0x4004, 0x3400);  // Another common port
}