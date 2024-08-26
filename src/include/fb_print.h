#ifndef FB_PRINT_H
#define FB_PRINT_H

#include <stddef.h>
#include <stdint.h>

// Function to print a string on the framebuffer
void fb_print(const char *str, size_t x, size_t y, uint32_t color);

// Function to print formatted data
void fb_printf(size_t x, size_t y, uint32_t color, const char *format, ...);

#endif // FB_PRINT_H
