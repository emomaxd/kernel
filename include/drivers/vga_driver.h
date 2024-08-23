#ifndef VGA_DRIVER_H
#define VGA_DRIVER_H

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VIDEO_MEMORY 0xb8000

// Struct representing a character cell in VGA text mode
typedef struct {
    unsigned char character;
    unsigned char color;
} VGAChar;

// Function prototypes
void outb(unsigned short port, unsigned char value);
void set_cursor_position(unsigned int row, unsigned int col);
void clear_screen(unsigned int color);
void print(const char* str, unsigned int color);
void print_at(const char* str, unsigned int color, unsigned int row, unsigned int col);

#endif
