#ifndef ARCH_H
#define ARCH_H

#include <stdint.h>
#include <stdbool.h>

void arch_init(void);

void arch_pause(void);
void arch_halt(bool interrupts);

void arch_disable_interrupts(void);
void arch_enable_interrupts(void);

#endif // ARCH_H