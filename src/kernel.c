#include "drivers/vga_driver.h"
#include "memory/memory.h"

// Entry point for the program
extern void kernel_main(void) {

    memory_init((void*)0, 0);
    // Example usage of the functions
    clear_screen(0x1F); // Clear screen with white text on blue background
    print_at("Welcome to my OS!", 0x1E, 1, 1); // Print message with yellow text on blue background at row 1, col 1

    while (1) {
        // Infinite loop to keep the program running
    }
}