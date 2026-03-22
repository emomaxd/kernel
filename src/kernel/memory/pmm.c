#include "pmm.h"
#include "limine.h"
#include "kprint.h"
#include "io.h"
#include <stdbool.h>

// Use memmap_request from hardware_info.c
extern volatile struct limine_memmap_request memmap_request;

// HHDM request — we need this for physical-to-virtual translation
__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

// HHDM offset provided by Limine
uint64_t hhdm_offset = 0;

// Bitmap for physical page allocation
// Each bit represents one 4KB page
// Max supported: 4GB = 1M pages = 128KB bitmap
#define MAX_PAGES (1024 * 1024)  // 4GB / 4KB
static uint8_t bitmap[MAX_PAGES / 8];

static uint64_t total_pages = 0;
static uint64_t used_pages = 0;
static uint64_t highest_addr = 0;

static inline void bitmap_set(uint64_t page) {
    bitmap[page / 8] |= (1 << (page % 8));
}

static inline void bitmap_clear(uint64_t page) {
    bitmap[page / 8] &= ~(1 << (page % 8));
}

static inline bool bitmap_test(uint64_t page) {
    return (bitmap[page / 8] >> (page % 8)) & 1;
}

void pmm_init(void) {
    if (!memmap_request.response || !hhdm_request.response) {
        return;
    }

    hhdm_offset = hhdm_request.response->offset;

    struct limine_memmap_response *memmap = memmap_request.response;

    // First: mark everything as used
    memset(bitmap, 0xFF, sizeof(bitmap));

    // Second: mark usable regions as free
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t base = entry->base;
            uint64_t length = entry->length;
            uint64_t end = base + length;

            if (end > highest_addr)
                highest_addr = end;

            // Align base up to page boundary
            base = (base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

            for (uint64_t addr = base; addr + PAGE_SIZE <= end; addr += PAGE_SIZE) {
                uint64_t page = addr / PAGE_SIZE;
                if (page < MAX_PAGES) {
                    bitmap_clear(page);
                    total_pages++;
                }
            }
        }
    }

    // Never allocate page 0 (null page)
    bitmap_set(0);

    used_pages = 0;
}

void *pmm_alloc_page(void) {
    uint64_t max_page = highest_addr / PAGE_SIZE;
    if (max_page > MAX_PAGES)
        max_page = MAX_PAGES;

    for (uint64_t i = 1; i < max_page; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            used_pages++;
            uint64_t phys = i * PAGE_SIZE;
            // Zero the page via HHDM
            void *virt = (void *)(phys + hhdm_offset);
            memset(virt, 0, PAGE_SIZE);
            return (void *)phys;
        }
    }
    return NULL;  // Out of memory
}

void pmm_free_page(void *addr) {
    uint64_t phys = (uint64_t)addr;
    uint64_t page = phys / PAGE_SIZE;
    if (page > 0 && page < MAX_PAGES && bitmap_test(page)) {
        bitmap_clear(page);
        if (used_pages > 0)
            used_pages--;
    }
}

uint64_t pmm_get_total_pages(void) { return total_pages; }
uint64_t pmm_get_used_pages(void)  { return used_pages; }
uint64_t pmm_get_free_pages(void)  { return total_pages - used_pages; }
uint64_t pmm_get_total_memory(void) { return highest_addr; }
