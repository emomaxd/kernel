#include "timer.h"
#include "io.h"
#include "irq.h"
#include "idt.h"
#include "process.h"

#define PIT_CMD  0x43
#define PIT_CH0  0x40

static volatile uint64_t ticks = 0;

static InterruptRegisters *timer_irq_handler(InterruptRegisters *regs) {
    ticks++;
    scheduler_tick();
    return regs;
}

void timer_init(void) {
    // PIT frequency: 1193182 Hz
    // Divisor for desired frequency
    uint16_t divisor = 1193182 / PIT_FREQUENCY;

    // Channel 0, lobyte/hibyte, rate generator mode
    outb(PIT_CMD, 0x36);
    outb(PIT_CH0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH0, (uint8_t)((divisor >> 8) & 0xFF));

    IRQ_installHandler(0, (handler_t)timer_irq_handler);
}

uint64_t timer_get_ticks(void) {
    return ticks;
}

void timer_sleep(uint64_t ms) {
    uint64_t target = ticks + ms;
    while (ticks < target) {
        __asm__ volatile("hlt");
    }
}
