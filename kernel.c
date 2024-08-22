void kernel_main(void) {
    char *video_memory = (char*) 0xB8000;
    const char *message = "Hello, Kernel!";
    
    for (int i = 0; message[i] != '\0'; i++) {
        video_memory[i * 2] = message[i];        // Character
        video_memory[i * 2 + 1] = 0x07;          // Attribute (white on black)
    }
    
    while (1) {
        // Infinite loop to prevent the kernel from exiting
    }
}
