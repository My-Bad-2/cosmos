#include <arch.h>
#include <limine.h>
#include <arch.h>
#include <stdio.h>

void kmain() {
	arch_init();

	printf("Hello, World!\n");

	arch_halt(true);
}