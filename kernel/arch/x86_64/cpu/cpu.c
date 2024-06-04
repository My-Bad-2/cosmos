#include <arch.h>
#include <cpu/cpu.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>

void cpu_init() {
	arch_disable_interrupts();

	gdt_init();
	idt_init();

	arch_enable_interrupts();
}