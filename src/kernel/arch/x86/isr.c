#include "isr.h"
#include "kprint.h"
#include "io.h"

char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD floating-point exception",
    "Virtualization exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Timer",
};

InterruptRegisters* ISR_handler(InterruptRegisters* regs) {
    if (regs->interrupt < 32) {
        kprint("Unhandled Interrupt",0, 300, 0xffffff);
		kprint("Exception",60, 300, 0xffffff);
        panic("\nCPU Panic");
    } else if (regs->interrupt > 32) {
		kprint("Interrupt",0, 140, 0xffffff);
        outb(0x60, 0);
    }

    return regs;
}