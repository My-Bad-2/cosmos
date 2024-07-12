#ifndef CPU_INTERRUPTS_H
#define CPU_INTERRUPTS_H

#include "uacpi/types.h"
#include <cpu/registers.h>
#include <uacpi/kernel_api.h>

#define IRQ_TO_INTERRUPT(x) ((x) + 0x20)

#define PLATFORM_INTERRUPT_BASE 0x20
#define PLATFORM_INTERRUPT_MAX 0xff

#define IRQ_TIMER IRQ_TO_INTERRUPT(0)
#define IRQ_KEYBOARD IRQ_TO_INTERRUPT(1)
#define IRQ_CASCADE_SIGNAL IRQ_TO_INTERRUPT(2)
#define IRQ_COM2 IRQ_TO_INTERRUPT(3)
#define IRQ_COM1 IRQ_TO_INTERRUPT(4)
#define IRQ_LPT2 IRQ_TO_INTERRUPT(5)
#define IRQ_FLOPPY_DISK IRQ_TO_INTERRUPT(6)
#define IRQ_LPT1 IRQ_TO_INTERRUPT(7)

#define IRQ_RTC IRQ_TO_INTERRUPT(8)
#define IRQ_INTEL_ACPI IRQ_TO_INTERRUPT(9)
#define IRQ_MOUSE IRQ_TO_INTERRUPT(12)
#define IRQ_CPU_COPROCESSOR IRQ_TO_INTERRUPT(13)
#define IRQ_PRIMARY_HARD_DISK IRQ_TO_INTERRUPT(14)
#define IRQ_SECONDARY_HARD_DISK IRQ_TO_INTERRUPT(15)

#define IRQ_SPURIOUS_INTERRUPT IRQ_LPT1

#define INTERRUPT_LOCAL_APIC_BASE 0xf0
#define INTERRUPT_APIC_SPURIOUS 0xf0
#define INTERRUPT_APIC_TIMER 0xf1
#define INTERRUPT_APIC_ERROR 0xf2
#define INTERRUPT_APIC_PMI 0xf3
#define INTERRUPT_IPI_GENERIC 0xf4
#define INTERRUPT_IPI_RESCHEDULE 0xf5
#define INTERRUPT_IPI_INTERRUPT 0xf6
#define INTERRUPT_IPI_HALT 0xf7

enum interrupt_trigger_mode {
	IRQ_TRIGGER_MODE_EDGE = 0,
	IRQ_TRIGGER_MODE_LEVEL = 1,
};

enum interrupt_polarity {
	IRQ_POLARITY_ACTIVE_HIGH = 0,
	IRQ_POLARITY_ACTIVE_LOW = 1,
};

typedef void (*interrupt_handler_t)(struct iframe* iframe);

struct interrupt_handler {
	interrupt_handler_t handler;
	uacpi_interrupt_handler uacpi_handler;
	void* ctx;
	int vector_id;
	bool reserved;
	bool eoi_first;
};

void interrupt_handler_init(void);

struct interrupt_handler* allocate_handler(int hint,
										   interrupt_handler_t handler);
void uacpi_allocate_interrupt_handler(uacpi_interrupt_handler handler,
									  int vector, void* ctx);
void deallocate_interrupt_handler(int vector);

void set_interrupt_mask(int vector);
void clear_interrupt_mask(int vector);

void call_interrupt_handler(struct iframe* iframe);

#endif	// CPU_INTERRUPTS_H