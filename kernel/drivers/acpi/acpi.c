#include <assert.h>
#include <drivers/acpi.h>
#include <drivers/acpi/madt.h>
#include <uacpi/uacpi.h>

void acpi_init(virt_addr_t rsdp_address) {
	uacpi_init_params uacpi_params = {
		.rsdp = (uacpi_phys_addr)from_higher_half(rsdp_address),
		.log_level = UACPI_LOG_INFO,
		.flags = 0,
	};

	assert(uacpi_initialize(&uacpi_params) == UACPI_STATUS_OK);

	madt_init();
}