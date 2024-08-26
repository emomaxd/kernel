#include "fb_print.h"
#include <stdarg.h>
#include <stdbool.h>

#include "font.h"

// Assuming these are defined elsewhere
extern uint32_t *framebuffer_ptr;
extern size_t framebuffer_width;
extern size_t framebuffer_height;
extern size_t framebuffer_pitch;

// Basic function to put a single character on the framebuffer
static void put_char(size_t x, size_t y, char c, uint32_t color) {
    // Ensure the character is within the printable range
    if (c < 32 || c > 127) {
        c = 32; // Default to space if out of range
    }

    // Get the font data for the character
    const uint8_t* glyph = font[(int)c];

    // Loop through each row (8 rows)
    for (size_t row = 0; row < 8; row++) {
        uint8_t bitmap = glyph[row];

        // Loop through each column (8 columns)
        for (size_t col = 0; col < 8; col++) {
            // Check if the bit is set in the bitmap
            if (bitmap & (1 << (7 - col))) {
                // Set the pixel in the framebuffer
                framebuffer_ptr[(y + row) * framebuffer_width + (x + col)] = color;
            }
        }
    }
}

// Function to print a string on the framebuffer
void fb_print(const char *str, size_t x, size_t y, uint32_t color) {
    while (*str) {
        put_char(x, y, *str++, color);
        x += 8; // Move to the next character position
        if (x >= framebuffer_width) {
            x = 0;
            y += 16; // Move to the next line
        }
    }
}

// Helper function to convert integers to string
static void int_to_str(int num, char *str, int base) {
    char digits[] = "0123456789ABCDEF";
    char buffer[32];
    int i = 0, j;
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    while (num) {
        buffer[i++] = digits[num % base];
        num /= base;
    }
    
    for (j = 0; j < i; j++) {
        str[j] = buffer[i - j - 1];
    }
    str[i] = '\0';
}

// Function to print formatted data on the framebuffer
void fb_printf(size_t x, size_t y, uint32_t color, const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    
    char *p = buffer;
    char *str;
    int i;
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd':
                    i = va_arg(args, int);
                    int_to_str(i, p, 10);
                    while (*p) p++;
                    break;
                case 's':
                    str = va_arg(args, char *);
                    while (*str) {
                        *p++ = *str++;
                    }
                    break;
                // Add other format specifiers here if needed
                default:
                    *p++ = '%';
                    *p++ = *format;
                    break;
            }
        } else {
            *p++ = *format;
        }
        format++;
    }
    *p = '\0';
    va_end(args);
    
    fb_print(buffer, x, y, color);
}
