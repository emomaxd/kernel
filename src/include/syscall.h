#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include "idt.h"

#define SYSCALL_WRITE  0
#define SYSCALL_EXIT   1
#define SYSCALL_GETPID 2

void syscall_init(void);
InterruptRegisters *syscall_handler(InterruptRegisters *regs);

#endif // SYSCALL_H
