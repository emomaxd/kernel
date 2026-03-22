#include "kernel.h"

__attribute__((used, section(".requests")))
volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests_start_marker")))
volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
volatile LIMINE_REQUESTS_END_MARKER;


void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    framebuffer_ptr = (uint32_t *)framebuffer->address;
    framebuffer_width = framebuffer->width;
    framebuffer_height = framebuffer->height;
    framebuffer_pitch = framebuffer->pitch / 4;

    // Phase 0: CPU tables
    GDT_init();
    IDT_init();

    // Phase 1: Physical memory manager
    pmm_init();

    // Phase 2: Virtual memory manager
    vmm_init();

    // Phase 3: Timer (PIT)
    timer_init();

    // Phase 4: Process management
    process_init();

    // Phase 5: Syscall interface
    syscall_init();

    // Phase 6: RAM filesystem
    ramfs_init();

    // Phase 7: Keyboard + Shell
    keyboard_init();
    shell_init();
    shell_run();

    hcf();
}
