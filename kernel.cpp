// Function to print a string to the video memory
static void print(const char* str, unsigned int color, unsigned int row, unsigned int col) {
    // Video memory starts at 0xb8000 for text mode
    unsigned char* video_memory = (unsigned char*)0xb8000;

    // Define the number of columns on the screen (80 for standard text mode)
    const unsigned int columns = 80;

    // Calculate the starting position in the video memory
    unsigned int position = (row * columns + col) * 2;

    // Iterate over the string and write each character to video memory
    while (*str) {
        // Write the character to the video memory
        video_memory[position] = *str;
        // Write the color attribute to the video memory
        video_memory[position + 1] = color;

        // Move to the next character position
        position += 2;
        str++;
    }
}

// Entry point for the program
extern "C" void main() {
    // Example usage of the print function
    // Print "Hello, World!" in white on a blue background at the top-left corner
    print("Hello, World!", 0x1F, 0, 0);

    // Halt the CPU (in a real OS, you would return or loop forever)
    while (1) {
        // Do nothing (infinite loop to keep the program running)
    }
}
