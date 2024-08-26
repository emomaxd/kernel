#include "draw_2d.h"

#include "limine.h"
#include "init.h"



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