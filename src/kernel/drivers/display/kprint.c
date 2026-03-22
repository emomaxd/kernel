#include "kprint.h"
#include <stdarg.h>
#include <stdbool.h>
#include "font.h"

// Assuming these are defined elsewhere
extern uint32_t *framebuffer_ptr;
extern size_t framebuffer_width;
extern size_t framebuffer_height;
extern size_t framebuffer_pitch;

// Function to convert integer to string
void itoa(int num, char *str, int base) {
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

// Function to convert string to integer (base 10)
int atoi(const char *str) {
    int result = 0;
    while (*str) {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

#define BACKGROUND_COLOR 0x000000
#define FONT_SCALE       2
#define FONT_SIZE        8
#define CHAR_PIXELS      (FONT_SIZE * FONT_SCALE)

/* Basic function to put a single character on the framebuffer */
static void put_char(size_t x, size_t y, char c, uint32_t color) {
    size_t row, col, sy, sx;
    uint32_t pixel;

    if (c < 32 || c > 127)
        c = ' ';

    if (c == ' ') {
        for (row = 0; row < CHAR_PIXELS; row++)
            for (col = 0; col < CHAR_PIXELS; col++)
                framebuffer_ptr[(y + row) * framebuffer_width + (x + col)] = BACKGROUND_COLOR;
        return;
    }

    const uint8_t *glyph = font[(int)c];

    for (row = 0; row < FONT_SIZE; row++) {
        uint8_t bitmap = glyph[row];
        for (col = 0; col < FONT_SIZE; col++) {
            pixel = (bitmap & (1 << (7 - col))) ? color : BACKGROUND_COLOR;
            for (sy = 0; sy < FONT_SCALE; sy++)
                for (sx = 0; sx < FONT_SCALE; sx++)
                    framebuffer_ptr[(y + row * FONT_SCALE + sy) * framebuffer_width +
                                   (x + col * FONT_SCALE + sx)] = pixel;
        }
    }
}

/* Function to print a string on the framebuffer */
void kprint(const char *str, size_t x, size_t y, uint32_t color) {
    while (*str) {
        put_char(x, y, *str++, color);
        x += CHAR_PIXELS;
        if (x >= framebuffer_width) {
            x = 0;
            y += CHAR_PIXELS;
        }
    }
}

// Function to print formatted data on the framebuffer
void kprintf(size_t x, size_t y, uint32_t color, const char *format, ...) {
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
                    itoa(i, p, 10);
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

    kprint(buffer, x, y, color);
}

// Function to print a hexadecimal number
void kprint_hex(uint32_t value, size_t x, size_t y, uint32_t color) {
    char buf[11];
    itoa(value, buf, 16);
    kprint("0x", x, y, color);
    x += 16; // Move to the next position for the number
    kprint(buf, x, y, color);
}

// Function to print a decimal number
void kprint_dec(uint32_t value, size_t x, size_t y, uint32_t color) {
    char buf[11];
    itoa(value, buf, 10);
    kprint(buf, x, y, color);
}
