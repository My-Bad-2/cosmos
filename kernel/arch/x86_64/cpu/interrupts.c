#include <cpu/idt.h>
#include <cpu/interrupts.h>
#include <cpu/pic.h>
#include <drivers/acpi/madt.h>
#include <drivers/ioapic.h>
#include <log.h>
#include <stdlib.h>

#define INTERRUPT_TO_IRQ(x) ((x) - 0x20)

struct interrupt_handler* interrupt_handlers = NULL;

bool is_handler_reserved(struct interrupt_handler* handler) {
	return (handler->reserved == true);
}

// clang-format off

bool reserve_interrupt_handler(struct interrupt_handler* handler) {
	if (is_handler_reserved(handler)) {
		return false;
	}

	return handler->reserved = true;
}

// clang-format on

bool reset_handler(struct interrupt_handler* handler) {
	bool ret = (bool)handler->handler;

	handler->handler = NULL;
	handler->uacpi_handler = NULL;
	handler->ctx = NULL;
	handler->reserved = false;
	handler->eoi_first = false;

	return ret;
}

void send_eoi(int vector) {
	pic_send_eoi(INTERRUPT_TO_IRQ(vector));
}

void interrupt_handler_init(void) {
	interrupt_handlers =
		calloc(PLATFORM_INTERRUPT_MAX + 1, sizeof(struct interrupt_handler));

	for (size_t i = 0; i < PLATFORM_INTERRUPT_BASE; i++) {
		reserve_interrupt_handler(&interrupt_handlers[i]);
	}
}

struct interrupt_handler* allocate_handler(int hint,
										   interrupt_handler_t handler) {
	if (hint < PLATFORM_INTERRUPT_BASE) {
		hint += IRQ_TO_INTERRUPT(0);
	}

	for (int i = hint; i <= PLATFORM_INTERRUPT_MAX; i++) {
		if (is_handler_reserved(&interrupt_handlers[i]) == false) {
			struct interrupt_handler* int_handler = &interrupt_handlers[i];
			reserve_interrupt_handler(int_handler);

			int_handler->handler = handler;
			int_handler->uacpi_handler = NULL;
			int_handler->ctx = NULL;
			int_handler->vector_id = i;
			int_handler->eoi_first = false;

			return int_handler;
		}
	}

	log_fatal("Out of Interrupt Handlers");
	return NULL;
}

void uacpi_allocate_interrupt_handler(uacpi_interrupt_handler handler,
									  int vector, void* ctx) {
	struct interrupt_handler* int_handler = &interrupt_handlers[vector];

	if (is_handler_reserved(int_handler)) {
		log_fatal("UACPI Interrupt handler is already reserved");
	}

	reserve_interrupt_handler(int_handler);

	int_handler->handler = NULL;
	int_handler->uacpi_handler = handler;
	int_handler->ctx = ctx;
	int_handler->vector_id = vector;
	int_handler->eoi_first = false;
}

void deallocate_interrupt_handler(int vector) {
	struct interrupt_handler* int_handler = &interrupt_handlers[vector];
	reset_handler(int_handler);
}

void set_interrupt_mask(int vector) {
	if (ioapic_is_enabled() && legacy_pic_enabled()) {
		ioapic_set_mask_isa_irq(INTERRUPT_TO_IRQ(vector));
	} else {
		pic_set_mask(INTERRUPT_TO_IRQ(vector));
	}
}

void clear_interrupt_mask(int vector) {
	if (ioapic_is_enabled() && legacy_pic_enabled()) {
		ioapic_clear_mask_isa_irq(INTERRUPT_TO_IRQ(vector));
	} else {
		pic_clear_mask(INTERRUPT_TO_IRQ(vector));
	}
}

void call_interrupt_handler(struct iframe* iframe) {
	struct interrupt_handler* handler = &interrupt_handlers[iframe->vector];

	if (is_handler_reserved(handler) == false) {
		log_fatal("Interrupt Handler %lu not reserved", iframe->vector);
	}

	if (handler->eoi_first) {
		send_eoi(handler->vector_id);
	}

	if (handler->uacpi_handler != NULL) {
		handler->uacpi_handler(handler->ctx);
	} else {
		handler->handler(iframe);
	}

	if (handler->eoi_first == false) {
		send_eoi(handler->vector_id);
	}
}