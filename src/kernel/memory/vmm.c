#include "vmm.h"
#include "pmm.h"
#include "io.h"
#include <stdbool.h>

extern uint64_t hhdm_offset;

static uint64_t *kernel_pml4 = NULL;

// Page table entry masks
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL
#define PTE_FLAGS_MASK 0xFFF0000000000FFFULL

static inline uint64_t *pte_to_virt(uint64_t entry) {
    return (uint64_t *)((entry & PTE_ADDR_MASK) + hhdm_offset);
}

// Get or create page table entry at given level
static uint64_t *vmm_get_next_level(uint64_t *table, uint64_t index, uint64_t flags, bool create) {
    if (table[index] & VMM_FLAG_PRESENT) {
        return pte_to_virt(table[index]);
    }

    if (!create)
        return NULL;

    void *new_page = pmm_alloc_page();
    if (!new_page)
        return NULL;

    table[index] = (uint64_t)new_page | flags | VMM_FLAG_PRESENT;
    return (uint64_t *)((uint64_t)new_page + hhdm_offset);
}

void vmm_map_page(uint64_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx   = (virt >> 21) & 0x1FF;
    uint64_t pt_idx   = (virt >> 12) & 0x1FF;

    // Intermediate levels always get write + user flags so leaf flags control access
    uint64_t intermediate_flags = VMM_FLAG_WRITE | (flags & VMM_FLAG_USER);

    uint64_t *pdpt = vmm_get_next_level(pml4, pml4_idx, intermediate_flags, true);
    uint64_t *pd   = vmm_get_next_level(pdpt, pdpt_idx, intermediate_flags, true);
    uint64_t *pt   = vmm_get_next_level(pd,   pd_idx,   intermediate_flags, true);

    pt[pt_idx] = (phys & PTE_ADDR_MASK) | flags | VMM_FLAG_PRESENT;
}

void vmm_unmap_page(uint64_t *pml4, uint64_t virt) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx   = (virt >> 21) & 0x1FF;
    uint64_t pt_idx   = (virt >> 12) & 0x1FF;

    uint64_t *pdpt = vmm_get_next_level(pml4, pml4_idx, 0, false);
    if (!pdpt) return;
    uint64_t *pd = vmm_get_next_level(pdpt, pdpt_idx, 0, false);
    if (!pd) return;
    uint64_t *pt = vmm_get_next_level(pd, pd_idx, 0, false);
    if (!pt) return;

    pt[pt_idx] = 0;

    // Invalidate TLB for this address
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

uint64_t vmm_virt_to_phys(uint64_t *pml4, uint64_t virt) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx   = (virt >> 21) & 0x1FF;
    uint64_t pt_idx   = (virt >> 12) & 0x1FF;

    uint64_t *pdpt = vmm_get_next_level(pml4, pml4_idx, 0, false);
    if (!pdpt) return 0;
    uint64_t *pd = vmm_get_next_level(pdpt, pdpt_idx, 0, false);
    if (!pd) return 0;
    uint64_t *pt = vmm_get_next_level(pd, pd_idx, 0, false);
    if (!pt) return 0;

    if (!(pt[pt_idx] & VMM_FLAG_PRESENT))
        return 0;

    return (pt[pt_idx] & PTE_ADDR_MASK) | (virt & 0xFFF);
}

uint64_t *vmm_get_kernel_pml4(void) {
    return kernel_pml4;
}

void vmm_switch_address_space(uint64_t *pml4) {
    uint64_t phys = (uint64_t)pml4;
    __asm__ volatile("mov %0, %%cr3" : : "r"(phys) : "memory");
}

void vmm_init(void) {
    // Read current CR3 (Limine's page tables)
    uint64_t old_cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(old_cr3));

    // Allocate new PML4
    kernel_pml4 = (uint64_t *)pmm_alloc_page();
    if (!kernel_pml4)
        return;

    uint64_t *pml4_virt = (uint64_t *)((uint64_t)kernel_pml4 + hhdm_offset);

    // Copy Limine's PML4 entries to preserve existing mappings
    // This includes HHDM, kernel higher-half, framebuffer, etc.
    uint64_t *old_pml4 = (uint64_t *)(old_cr3 + hhdm_offset);
    for (int i = 0; i < 512; i++) {
        pml4_virt[i] = old_pml4[i];
    }

    // Switch to our page tables
    vmm_switch_address_space(kernel_pml4);
}

uint64_t *vmm_create_address_space(void) {
    uint64_t *new_pml4 = (uint64_t *)pmm_alloc_page();
    if (!new_pml4)
        return NULL;

    uint64_t *new_virt = (uint64_t *)((uint64_t)new_pml4 + hhdm_offset);
    uint64_t *kernel_virt = (uint64_t *)((uint64_t)kernel_pml4 + hhdm_offset);

    // Zero user-space half (entries 0-255)
    for (int i = 0; i < 256; i++) {
        new_virt[i] = 0;
    }

    // Copy kernel-space half (entries 256-511)
    for (int i = 256; i < 512; i++) {
        new_virt[i] = kernel_virt[i];
    }

    return new_pml4;
}

// Simple kernel heap using page allocator
// This replaces the static buddy allocator for dynamic allocations

// Simple bump allocator on top of page allocation
static uint64_t heap_current = 0;
static uint64_t heap_end = 0;
#define KHEAP_START 0xFFFFFFFF90000000ULL

void *kmalloc(size_t size) {
    // Align to 16 bytes
    size = (size + 15) & ~15;

    // Add header for tracking size
    size += 16;

    if (heap_current == 0) {
        heap_current = KHEAP_START;
        heap_end = KHEAP_START;
    }

    // Need more pages?
    while (heap_current + size > heap_end) {
        void *page = pmm_alloc_page();
        if (!page)
            return NULL;
        uint64_t *pml4_virt = (uint64_t *)((uint64_t)kernel_pml4 + hhdm_offset);
        vmm_map_page(pml4_virt, heap_end, (uint64_t)page, VMM_FLAG_PRESENT | VMM_FLAG_WRITE);
        heap_end += PAGE_SIZE;
    }

    uint64_t *header = (uint64_t *)heap_current;
    *header = size;
    heap_current += size;

    return (void *)((uint64_t)header + 16);
}

void kfree(void *ptr) {
    // Simple bump allocator — kfree is a no-op for now
    // A real slab/free-list allocator can be added later
    (void)ptr;
}
