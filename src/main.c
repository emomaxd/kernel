#include "kernel.h"



// Set the base revision to 2, as recommended, since this is the latest
// base revision described by the Limine boot protocol specification.
// See the specification for further information.

__attribute__((used, section(".requests")))
volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimize them away, so they should be made
// volatile or equivalent and accessed at least once, or marked as used
// with the "used" attribute as done here.

__attribute__((used, section(".requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as needed.

__attribute__((used, section(".requests_start_marker")))
volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
volatile LIMINE_REQUESTS_END_MARKER;




// Kernel entry point
void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
    

    //idt_init();

    // Fetch the first framebuffer
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    framebuffer_ptr = (uint32_t *)framebuffer->address;
    framebuffer_width = framebuffer->width;
    framebuffer_height = framebuffer->height;
    framebuffer_pitch = framebuffer->pitch / 4; // in pixels
/*
        // Define rectangle parameters
    size_t rect_x = 500;
    size_t rect_y = 500;
    size_t rect_width = 200;
    size_t rect_height = 100;
    uint32_t rect_color = 0xFFFFFF; // Red

    // Draw a rectangle on the framebuffer
    draw_rect(rect_x, rect_y, rect_width, rect_height, rect_color);
    */

    __asm__ ("sti");
    GDT_init();


    IDT_init();

    keyboard_init();

    shell_init();
    shell_run();

    //print_memory_info();
    
    hcf();
}
