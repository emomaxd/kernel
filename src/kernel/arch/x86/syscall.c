#include "syscall.h"
#include "kprint.h"
#include "process.h"
#include "idt.h"

// sys_write(fd, buf, len)
static uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t len) {
    (void)fd;
    char *str = (char *)buf;
    if (len > 4096) len = 4096;

    // Print to framebuffer at a fixed position for now
    char tmp[128];
    size_t i;
    for (i = 0; i < len && i < 127; i++) {
        tmp[i] = str[i];
    }
    tmp[i] = '\0';
    kprint(tmp, 0, 400, 0x42f548);
    return len;
}

static uint64_t sys_exit(uint64_t code) {
    (void)code;
    process_exit();
    return 0;  // never reached
}

static uint64_t sys_getpid(void) {
    process_t *proc = process_get_current();
    return proc ? proc->pid : 0;
}

InterruptRegisters *syscall_handler(InterruptRegisters *regs) {
    uint64_t num = regs->rax;
    uint64_t ret = 0;

    switch (num) {
        case SYSCALL_WRITE:
            ret = sys_write(regs->rdi, regs->rsi, regs->rdx);
            break;
        case SYSCALL_EXIT:
            ret = sys_exit(regs->rdi);
            break;
        case SYSCALL_GETPID:
            ret = sys_getpid();
            break;
        default:
            ret = (uint64_t)-1;
            break;
    }

    regs->rax = ret;
    return regs;
}

void syscall_init(void) {
    // Set DPL=3 on int 0x80 so user mode can trigger it
    // The IDT entry flags field: bits 13-14 = DPL
    extern IDTEntry idt[IDT_ENTRY_COUNT];
    // Current flags have DPL=0, we need DPL=3
    // IDT_ATTR_PRESENT (0x8000) | IDT_ATTR_INTERRUPT_GATE (0x0e00) = 0x8e00
    // For DPL=3: 0x8e00 | (3 << 13) = 0x8e00 | 0x6000 = 0xee00
    idt[0x80].flags = 0xEE00;
}
