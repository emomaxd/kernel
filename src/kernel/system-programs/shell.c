#include "shell.h"
#include "keyboard.h"
#include "kprint.h"
#include "string.h"

#define SHELL_PROMPT ">"
#define SHELL_PROMPT_LENGTH 1
#define SHELL_BUFFER_SIZE 256
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8

static char command_buffer[SHELL_BUFFER_SIZE];
static uint8_t command_index = 0;
static uint16_t cursor_x = SHELL_PROMPT_LENGTH * CHAR_WIDTH;
static uint16_t cursor_y = 0;

// Forward declarations
static void handle_backspace();
static void handle_enter();
static void handle_tab();
static void print_command();
static void move_cursor_horizontal(uint16_t length);
static void move_cursor_vertical(uint16_t lines);
static void execute_command(const char *command);

static void handle_backspace() {
    if (command_index > 0) {
        // Decrement command index and clear the character from buffer
        command_buffer[--command_index] = '\0';
        
        // Move cursor left to the position of the last character
        move_cursor_horizontal(-1);

        // Clear the character from the screen
        kprint("        ", cursor_x, cursor_y, 0x000000); // Print space to clear previous character
    }
}

static void handle_enter() {
    command_buffer[command_index] = '\0'; // Null-terminate the command

    // Execute the command
    move_cursor_vertical(1);
    execute_command(command_buffer);

    // Reset command buffer
    memset(command_buffer, 0, sizeof(command_buffer));
    command_index = 0;

    // Move cursor down and print prompt
    move_cursor_vertical(1);
    kprint(SHELL_PROMPT, 0, cursor_y, 0xFFFFFF);
    cursor_x = SHELL_PROMPT_LENGTH * CHAR_WIDTH;
}

static void handle_tab() {
    const char *tab_spaces = "    ";
    size_t tab_len = strlen(tab_spaces);
    if (command_index + tab_len < SHELL_BUFFER_SIZE) {
        strncpy(&command_buffer[command_index], tab_spaces, tab_len);
        command_index += tab_len;
        kprint(tab_spaces, cursor_x, cursor_y, 0xFFFFFF);
        move_cursor_horizontal(tab_len);
    }
}

static void print_command() {
    kprint(command_buffer, cursor_x, cursor_y, 0xFFFFFF);
    move_cursor_horizontal(strlen(command_buffer));
}

static void move_cursor_horizontal(uint16_t lines) {
    cursor_x += lines * CHAR_WIDTH;
}

static void move_cursor_vertical(uint16_t lines) {
    cursor_y += lines * CHAR_HEIGHT;
    kprint("\n", 0, cursor_y, 0xFFFFFF);
}

static void execute_command(const char *command) {
    if (strcmp(command, "hello") == 0) {
        kprint("\nHello, world!", 0, cursor_y, 0xFFFFFF);
    } else if (strcmp(command, "clear") == 0) {
        kprint("\n", 0, cursor_y, 0xFFFFFF);
    } else if (strcmp(command, "reboot") == 0) {
        kprint("\nRebooting system...", 0, cursor_y, 0xFFFFFF);
    } else if (strcmp(command, "poweroff") == 0) {
        kprint("\nPowering off...", 0, cursor_y, 0xFFFFFF);
    } else {
        kprint("\nUnknown command", 0, cursor_y, 0xFFFFFF);
    }
}

void shell_init() {
    kprint(SHELL_PROMPT, 0, 0, 0xFFFFFF);
    command_index = 0;
    cursor_x = SHELL_PROMPT_LENGTH * CHAR_WIDTH;
    cursor_y = 0;
}

void shell_run() {
    while (1) {
        for (uint8_t scancode = 0; scancode < KEY_STATE_SIZE; ++scancode) {
            if (is_key_pressed(scancode)) {
                char key = scancode_to_char(scancode);
                switch (key) {
                    case '\b':
                        handle_backspace();
                        break;
                    case '\n':
                        handle_enter();
                        break;
                    case '\t':
                        handle_tab();
                        break;
                    default:
                        if (command_index < SHELL_BUFFER_SIZE - 1) {
                            command_buffer[command_index++] = key;
                            command_buffer[command_index] = '\0';
                            kprint(&key, cursor_x, cursor_y, 0xFFFFFF);
                            move_cursor_horizontal(1);
                        }
                        break;
                }
                update_key_state(scancode, 0);
            }
        }
    }
}
