#include <arch.h>

void arch_init() {}

void arch_pause() {
	asm volatile("pause");
}

void arch_halt(bool interrupts) {
	if (interrupts) {
		while (true) {
			arch_pause();
		}
	} else {
		while (true) {
			arch_disable_interrupts();
			arch_pause();
		}
	}
}

void arch_disable_interrupts() {
	asm volatile("cli");
}

void arch_enable_interrupts() {
    asm volatile("sti");
}