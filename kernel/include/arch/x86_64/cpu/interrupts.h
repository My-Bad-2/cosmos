#ifndef CPU_INTERRUPTS_H
#define CPU_INTERRUPTS_H

#include <cpu/registers.h>

typedef void (*interrupt_handler_t)(iframe_t* iframe);

void interrupt_handler_init(void);

int allocate_interrupt_handler(interrupt_handler_t handler);
void allocate_irq_handler(int irq_number, interrupt_handler_t handler);
void call_interrupt_handler(iframe_t* iframe);

#endif // CPU_INTERRUPTS_H