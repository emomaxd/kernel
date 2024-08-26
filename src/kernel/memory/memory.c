#include "memory.h"

#define HEAP_SIZE 0x100000 // 1 MB heap size
#define MIN_BLOCK_SIZE 0x1000 // 4 KB minimum block size

typedef struct block {
    struct block *next;
    size_t size;
} block_t;

static uint8_t heap[HEAP_SIZE];

static block_t *free_lists[HEAP_SIZE / MIN_BLOCK_SIZE + 1];



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



void init_malloc(void) {
    block_t *initial_block = (block_t *)heap;
    initial_block->size = HEAP_SIZE;
    initial_block->next = NULL;

    // Place the block in the highest free list.
    free_lists[HEAP_SIZE / MIN_BLOCK_SIZE] = initial_block;
}

static int get_list_index(size_t size) {
    int index = 0;
    while (MIN_BLOCK_SIZE << index < size) {
        index++;
    }
    return index;
}

// Split a block into two buddies.
static block_t *split_block(block_t *block, int target_index) {
    int current_index = get_list_index(block->size);
    while (current_index > target_index) {
        block_t *buddy = (block_t *)((uint8_t *)block + (block->size >> 1));
        buddy->size = block->size >> 1;
        buddy->next = free_lists[--current_index];
        free_lists[current_index] = buddy;

        block->size >>= 1;
    }
    return block;
}

void *malloc(size_t size) {
    size = (size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : size;
    int index = get_list_index(size);

    // Find the smallest available block that fits.
    while (index <= HEAP_SIZE / MIN_BLOCK_SIZE && free_lists[index] == NULL) {
        index++;
    }

    if (index > HEAP_SIZE / MIN_BLOCK_SIZE) {
        return NULL; // Out of memory
    }

    // Split blocks until the target block size is reached.
    block_t *block = free_lists[index];
    free_lists[index] = block->next;
    block = split_block(block, get_list_index(size));

    return (void *)((uint8_t *)block + sizeof(block_t));
}

static void coalesce_block(block_t *block) {
    int index = get_list_index(block->size);
    uint8_t *block_addr = (uint8_t *)block;

    while (index < HEAP_SIZE / MIN_BLOCK_SIZE) {
        // Calculate the buddy's address.
        uint8_t *buddy_addr = (uint8_t *)((uintptr_t)block_addr ^ block->size);

        // Check if the buddy is in the free list.
        block_t *prev = NULL;
        block_t *current = free_lists[index];
        while (current != NULL) {
            if ((uint8_t *)current == buddy_addr) {
                // Remove buddy from the free list.
                if (prev == NULL) {
                    free_lists[index] = current->next;
                } else {
                    prev->next = current->next;
                }

                // Merge the blocks.
                if (buddy_addr < block_addr) {
                    block_addr = buddy_addr;
                }
                block = (block_t *)block_addr;
                block->size <<= 1;
                index++;
                break;
            }
            prev = current;
            current = current->next;
        }

        if (current == NULL) {
            break; // No buddy found, stop coalescing.
        }
    }

    // Add the coalesced block to the free list.
    block->next = free_lists[index];
    free_lists[index] = block;
}

void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    block_t *block = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    coalesce_block(block);
}
