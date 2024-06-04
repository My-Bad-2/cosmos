#include "arch.h"
#include <cpu/registers.h>
#include <log.h>

void exception_handler(iframe_t* frame) {
	(void)frame;

	arch_disable_interrupts();

	log_fatal("Exception occurred!");

	arch_enable_interrupts();
}

void nmi_handler(iframe_t* frame) {
	(void)frame;

	log_fatal("NMI occurred!");
}