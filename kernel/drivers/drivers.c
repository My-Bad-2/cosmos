#include <drivers/acpi.h>
#include <drivers/drivers.h>
#include <drivers/timer.h>
#include <kernel.h>

extern void arch_drivers_init(void);

void drivers_init(void) {
	acpi_init((virt_addr_t)rsdp_request.response->address);
	arch_drivers_init();
	timer_init();
}