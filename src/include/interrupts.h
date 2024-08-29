#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

void init_interrupts();
void isr_handler(uint64_t vector);
void irq_handler(uint64_t irq);

#endif // INTERRUPTS_H
