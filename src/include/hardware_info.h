// hardware_info.h

#ifndef HARDWARE_INFO_H
#define HARDWARE_INFO_H

#include <stdint.h>

// Structure to hold memory information
typedef struct memory_info {
    uint64_t total_memory;
    uint64_t available_memory;
    // You can add more fields here as needed, like reserved memory, etc.
} memory_info_t;

// Function prototypes
memory_info_t get_memory_info();
void print_memory_info();

#endif // HARDWARE_INFO_H
