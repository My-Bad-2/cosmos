#ifndef CPU_INTERRUPTS_H
#define CPU_INTERRUPTS_H

#include <cpu/registers.h>

#define PLATFORM_INTERRUPT_BASE 0x20
#define PLATFORM_INTERRUPT_MAX 0xff

#define INTERRUPT_LOCAL_APIC_BASE 0xf0
#define INTERRUPT_APIC_SPURIOUS 0xf0
#define INTERRUPT_APIC_TIMER 0xf1
#define INTERRUPT_APIC_ERROR 0xf2
#define INTERRUPT_APIC_PMI 0xf3
#define INTERRUPT_IPI_GENERIC 0xf4
#define INTERRUPT_IPI_RESCHEDULE 0xf5
#define INTERRUPT_IPI_INTERRUPT 0xf6
#define INTERRUPT_IPI_HALT 0xf7

typedef uint32_t (*interrupt_handler_t)(struct iframe* iframe, void* ctx);

void interrupt_handler_init(void);

int allocate_interrupt_handler(interrupt_handler_t handler, void* ctx);
void allocate_interrupt_handler_at(interrupt_handler_t handler, int vector,
								   void* ctx);
void allocate_irq_handler(int irq_number, interrupt_handler_t handler,
						  void* ctx);
void call_interrupt_handler(struct iframe* iframe);
void clear_interrupt_handler(int vector);

void set_interrupt_mask(int vector);
void clear_interrupt_mask(int vector);

#endif	// CPU_INTERRUPTS_H