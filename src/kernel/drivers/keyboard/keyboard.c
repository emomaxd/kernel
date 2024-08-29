#include "keyboard.h"
#include "irq.h"
#include "kprint.h"



static uint8_t key_state[KEY_STATE_SIZE] = {0};

char scancode_to_char(uint8_t scancode) {
    // Simple US QWERTY layout map for scancodes 0x02 to 0x0D
    static char keymap[256] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0,  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0,  ' ', 0
    };

    if (scancode < sizeof(keymap)) {
        return keymap[scancode];
    }
    return 0;
}

void update_key_state(uint8_t scancode, uint8_t pressed) {
    if (scancode < KEY_STATE_SIZE) {
        key_state[scancode] = pressed;
    }
}

uint8_t is_key_pressed(uint8_t scancode) {
    if (scancode < KEY_STATE_SIZE) {
        return key_state[scancode];
    }
    return 0;
}

InterruptRegisters* keyboard_handler(InterruptRegisters* regs) {
    uint8_t scancode = inb(KBD_DATA_PORT);

    if (scancode & 0x80) {
        // Key released
        update_key_state(scancode & 0x7F, 0);
    } else {
        // Key pressed
        update_key_state(scancode, 1);
    }

    return regs;
}

void keyboard_init() {
    IRQ_installHandler(1, keyboard_handler);
}
