#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

void pmm_init(void);
void *pmm_alloc_page(void);
void pmm_free_page(void *addr);

uint64_t pmm_get_total_pages(void);
uint64_t pmm_get_used_pages(void);
uint64_t pmm_get_free_pages(void);
uint64_t pmm_get_total_memory(void);

#endif // PMM_H
