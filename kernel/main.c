#include <arch.h>
#include <limine.h>
#include <arch.h>
#include <log.h>

void kmain() {
	arch_init();

	log_info("Hello World!");

	arch_halt(true);
}