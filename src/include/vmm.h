#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>

#define VMM_FLAG_PRESENT  (1ULL << 0)
#define VMM_FLAG_WRITE    (1ULL << 1)
#define VMM_FLAG_USER     (1ULL << 2)
#define VMM_FLAG_NOEXEC   (1ULL << 63)

#define KERNEL_VIRT_BASE  0xFFFFFFFF80000000ULL
#define HHDM_OFFSET       0xFFFF800000000000ULL

void vmm_init(void);
void vmm_map_page(uint64_t *pml4, uint64_t virt, uint64_t phys, uint64_t flags);
void vmm_unmap_page(uint64_t *pml4, uint64_t virt);
uint64_t vmm_virt_to_phys(uint64_t *pml4, uint64_t virt);
uint64_t *vmm_get_kernel_pml4(void);
uint64_t *vmm_create_address_space(void);
void vmm_switch_address_space(uint64_t *pml4);

// Kernel heap
void *kmalloc(size_t size);
void kfree(void *ptr);

// Helper to convert physical to virtual (HHDM)
static inline void *phys_to_virt(uint64_t phys) {
    return (void *)(phys + HHDM_OFFSET);
}

static inline uint64_t virt_to_phys_direct(void *virt) {
    return (uint64_t)virt - HHDM_OFFSET;
}

#endif // VMM_H
