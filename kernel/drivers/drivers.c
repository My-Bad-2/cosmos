#include <drivers/acpi.h>
#include <drivers/drivers.h>
#include <drivers/timer.h>
#include <kernel.h>

void drivers_init(void) {
	timer_init();
	acpi_init((virt_addr_t)rsdp_request.response->address);
}