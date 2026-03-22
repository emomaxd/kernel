#include "irq.h"
#include "pic.h"
#include "io.h"
#include "kprint.h"

// 16 hardware IRQs (IRQ0-15 = int 32-47)
void* IRQ_handlers[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void IRQ_installHandler(int irq, handler_t handler) {
	if (irq >= 0 && irq < 16)
		IRQ_handlers[irq] = handler;
}

void IRQ_uninstallHandler(int irq) {
	if (irq >= 0 && irq < 16)
		IRQ_handlers[irq] = 0;
}

InterruptRegisters* IRQ_handler(InterruptRegisters* regs) {
	int irq = regs->interrupt - 32;

	if (irq >= 0 && irq < 16) {
		handler_t handler = IRQ_handlers[irq];
		if (handler)
			handler(regs);
		PIC_eoi(irq);
	}

	return regs;
}
