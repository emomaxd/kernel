#include "memory/memory.h"

#define HEAP_SIZE 8192  // Define a heap size for testing

static unsigned char *heap_start = NULL;
static unsigned char *heap_end = NULL;
static unsigned char *heap_next = NULL;

// Initialize the memory heap
void memory_init(void *heap_start_param, size_t heap_size) {
    heap_start = (unsigned char *)heap_start_param;
    heap_end = heap_start + heap_size;
    heap_next = heap_start;
}

// Allocate memory from the heap
void *memory_alloc(size_t size) {
    if (heap_start == NULL || heap_next + size > heap_end) {
        return NULL;  // Out of memory
    }

    void *allocated_memory = heap_next;
    heap_next += size;
    return allocated_memory;
}

// Free memory (does nothing in this simple implementation)
void memory_free(void *ptr) {
    // In a simple implementation like this, we do not manage free memory.
    // Memory freeing would require a more sophisticated approach.
}
