#include "drivers/vga_driver.h"

// Global cursor position
static unsigned int cursor_row = 0;
static unsigned int cursor_col = 0;

// Write a byte to an I/O port
void outb(unsigned short port, unsigned char value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Function to move the cursor to a specific position
void set_cursor_position(unsigned int row, unsigned int col) {
    unsigned int position = row * VGA_WIDTH + col;

    // Send the low byte of the position to the VGA control register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));

    // Send the high byte of the position to the VGA control register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

// Function to clear the screen
void clear_screen(unsigned int color) {
    VGAChar* video_memory = (VGAChar*)VIDEO_MEMORY;

    for (unsigned int row = 0; row < VGA_HEIGHT; row++) {
        for (unsigned int col = 0; col < VGA_WIDTH; col++) {
            video_memory[row * VGA_WIDTH + col].character = ' ';
            video_memory[row * VGA_WIDTH + col].color = color;
        }
    }

    // Reset the cursor position
    cursor_row = 0;
    cursor_col = 0;
    set_cursor_position(cursor_row, cursor_col);
}

// Function to print a string at the current cursor position
void print(const char* str, unsigned int color) {
    VGAChar* video_memory = (VGAChar*)VIDEO_MEMORY;

    while (*str) {
        if (*str == '\n') {
            cursor_row++;
            cursor_col = 0;
        } else {
            video_memory[cursor_row * VGA_WIDTH + cursor_col].character = *str;
            video_memory[cursor_row * VGA_WIDTH + cursor_col].color = color;
            cursor_col++;

            if (cursor_col >= VGA_WIDTH) {
                cursor_col = 0;
                cursor_row++;
            }
        }

        if (cursor_row >= VGA_HEIGHT) {
            // Implement scrolling (optional, not covered in this example)
            cursor_row = VGA_HEIGHT - 1;
        }

        set_cursor_position(cursor_row, cursor_col);
        str++;
    }
}

// Function to set the cursor to a specific position and print text
void print_at(const char* str, unsigned int color, unsigned int row, unsigned int col) {
    cursor_row = row;
    cursor_col = col;
    print(str, color);
}
