#include <assert.h>
#include <cpu/idt.h>
#include <cpu/interrupts.h>
#include <cpu/pic.h>
#include <log.h>
#include <memory/heap.h>

struct interrupt_handler {
	interrupt_handler_t handler;
	int vector_id;
	bool reserved;
};

struct interrupt_handler* interrupt_handlers = NULL;

void interrupt_handler_init(void) {
	interrupt_handlers =
		heap_malloc(sizeof(struct interrupt_handler) * IDT_MAX_ENTRY);

	for (int i = 0; i < 32; ++i) {
		interrupt_handlers[i].handler = NULL;
		interrupt_handlers[i].reserved = true;
		interrupt_handlers[i].vector_id = i;
	}
}

void allocate_irq_handler(int irq_number, interrupt_handler_t handler) {
	assert(irq_number >= 32 && irq_number <= 48);

	if (interrupt_handlers[irq_number].reserved) {
		log_warn("IRQ %d is already allocated!", irq_number);
		return;
	}

	interrupt_handlers[irq_number].handler = handler;
	interrupt_handlers[irq_number].vector_id = irq_number;
	interrupt_handlers[irq_number].reserved = true;
}

int allocate_interrupt_handler(interrupt_handler_t handler) {
	for (int i = 49; i < IDT_MAX_ENTRY; ++i) {
		if (interrupt_handlers[i].reserved) {
			continue;
		}

		interrupt_handlers[i].handler = handler;
		interrupt_handlers[i].vector_id = i;
		interrupt_handlers[i].reserved = true;

		return i;
	}

	log_fatal("Out of interrupt handlers!");
	return 0;
}

void allocate_interrupt_handler_at(interrupt_handler_t handler, int vector) {
	if (interrupt_handlers[vector].reserved) {
		log_fatal("Interrupt handler is reserved.");
	}

	interrupt_handlers[vector].handler = handler;
	interrupt_handlers[vector].vector_id = vector;
	interrupt_handlers[vector].reserved = true;
}

void call_interrupt_handler(struct iframe* iframe) {
	if (interrupt_handlers[iframe->vector].handler == NULL) {
		log_fatal("Interrupt handler not allocated.");
	}

	if (!interrupt_handlers[iframe->vector].reserved) {
		log_fatal("Interrupt handler not allocated.");
	}

	interrupt_handlers[iframe->vector].handler(iframe);
}

void set_interrupt_mask(int vector) {
	pic_set_mask(vector);
}

void clear_interrupt_mask(int vector) {
	pic_clear_mask(vector);
}