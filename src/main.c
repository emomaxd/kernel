#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

// Set the base revision to 2, as recommended, since this is the latest
// base revision described by the Limine boot protocol specification.
// See the specification for further information.

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimize them away, so they should be made
// volatile or equivalent and accessed at least once, or marked as used
// with the "used" attribute as done here.

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as needed.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Port I/O functions
static inline void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

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

// Halt and catch fire function
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

// ACPI shutdown function
static inline void acpi_shutdown(void) {
    outw(0xB004, 0x2000);  // Common ACPI shutdown port

    // Alternative ports if the first doesn't work
    outw(0x604, 0x2000);   // Common for QEMU
    outw(0x4004, 0x3400);  // Another common port
}

// Draw a filled rectangle on the framebuffer
void draw_rect(size_t x, size_t y, size_t width, size_t height, uint32_t color) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t *fb_ptr = (uint32_t *)framebuffer->address;
    size_t pitch = framebuffer->pitch / 4; // in pixels
    size_t fb_width = framebuffer->width;
    size_t fb_height = framebuffer->height;

    // Clip the rectangle to fit within the framebuffer bounds
    if (x >= fb_width || y >= fb_height) return; // Rectangle is completely outside the framebuffer
    if (x + width > fb_width) width = fb_width - x;
    if (y + height > fb_height) height = fb_height - y;

    // Draw the rectangle
    for (size_t row = y; row < y + height; row++) {
        for (size_t col = x; col < x + width; col++) {
            fb_ptr[row * pitch + col] = color;
        }
    }
}

// Kernel entry point
void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Define rectangle parameters
    size_t rect_x = 50;
    size_t rect_y = 50;
    size_t rect_width = 200;
    size_t rect_height = 100;
    uint32_t rect_color = 0xFF0000; // Red

    // Draw a rectangle on the framebuffer
    draw_rect(rect_x, rect_y, rect_width, rect_height, rect_color);

    // Keyboard loop
    while (true) {
        uint8_t scancode = inb(0x60); // Read the keyboard scancode

        if (scancode == 0x01) { // 0x01 is the scancode for the ESC key
            acpi_shutdown(); // Shutdown the computer
        }
    }

    // If shutdown fails, halt the CPU in an infinite loop
    hcf();
}
