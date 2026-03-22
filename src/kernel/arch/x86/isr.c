#include "isr.h"
#include "kprint.h"
#include "io.h"
#include "syscall.h"

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
};

InterruptRegisters* ISR_handler(InterruptRegisters* regs) {
    if (regs->interrupt == 0x80) {
        // Syscall from user mode
        return syscall_handler(regs);
    }

    if (regs->interrupt < 32) {
        kprint("Exception: ", 0, 300, 0xFF4444);
        if (regs->interrupt < 32) {
            kprint(exception_messages[regs->interrupt], 88, 300, 0xFF4444);
        }
        kprint("System halted", 0, 320, 0xFF4444);
        panic("CPU Exception");
    }

    return regs;
}
