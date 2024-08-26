#include "hardware_info.h"
#include <limine.h>
#include <stddef.h>

#include "kprint.h"

// Limine memory map request
__attribute__((used, section(".requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

memory_info_t get_memory_info() {
    memory_info_t mem_info = {0}; // Initialize the structure with zeros

    // Check if the memory map request has been initialized
    if (!memmap_request.response) {
        return mem_info; // Return with zeros if not initialized
    }

    // Access the memory map response
    struct limine_memmap_response *response = memmap_request.response;

    // Iterate through memory map entries to gather total and available memory
    for (size_t i = 0; i < response->entry_count; i++) {
        struct limine_memmap_entry *entry = &response->entries[i];

        // Calculate total memory
        mem_info.total_memory += entry->length;

        // Calculate available memory based on type
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            mem_info.available_memory += entry->length;
        }
    }

    return mem_info; // Return the populated structure
}

void print_memory_info() {
    memory_info_t mem_info = get_memory_info();

    // Check if the memory map response was valid
    if (mem_info.total_memory == 0) {
        kprint("Memory map request not initialized.", 0, 0, 0xFFFFFF);
        return;
    }

    // Convert memory values to strings in MB
    char total_memory_str[32];
    char available_memory_str[32];
    
    // Convert bytes to megabytes (MB)
    uint64_t total_memory_mb = mem_info.total_memory / (1024 * 1024);
    uint64_t available_memory_mb = mem_info.available_memory / (1024 * 1024);

    total_memory_mb /= 1024 * 1024 * 1024;
    available_memory_mb /= 1024 * 1024 * 1024;

    // Use itoa to convert to strings
    itoa(total_memory_mb, total_memory_str, 10);
    itoa(available_memory_mb, available_memory_str, 10);

    // Print the results
    kprint("Total memory: ", 0, 0, 0xFFFFFF);
    kprint(total_memory_str, 160, 0, 0xFFFFFF);
    kprint(" MB", 320, 0, 0xFFFFFF);

    kprint("Available memory: ", 0, 10, 0xFFFFFF);
    kprint(available_memory_str, 200, 10, 0xFFFFFF);
    kprint(" MB", 320, 10, 0xFFFFFF);
}
