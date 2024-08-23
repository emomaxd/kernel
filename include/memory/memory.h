#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void memory_init(void *heap_start, size_t heap_size);
void *memory_alloc(size_t size);
void memory_free(void *ptr);

#endif // MEMORY_H
