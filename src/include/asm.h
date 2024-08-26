#ifndef ASM_H
#define ASM_H

#include <stdint.h>

// Halt and catch fire function
void hcf(void);

// Port I/O functions
void outw(uint16_t port, uint16_t value);
uint8_t inb(uint16_t port);

// ACPI shutdown function
void acpi_shutdown(void);

#endif // ASM_H
