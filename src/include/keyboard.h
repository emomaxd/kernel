#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "idt.h"

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT (KBD_DATA_PORT + 4) // 0x64
#define KEY_STATE_SIZE 256

void kprint_buffer(uint16_t x, uint16_t y, uint32_t color);
char scancode_to_char(uint8_t scancode);

InterruptRegisters* keyboard_handler(InterruptRegisters* regs);

void keyboard_init(void);
uint8_t is_key_pressed(uint8_t scancode);
void update_key_state(uint8_t scancode, uint8_t pressed);

#endif // KEYBOARD_H
