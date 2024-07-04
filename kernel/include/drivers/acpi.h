#ifndef DRIVERS_ACPI_H
#define DRIVERS_ACPI_H

#include <memory/addr.h>
#include <stddef.h>
#include <stdint.h>

void acpi_init(virt_addr_t rsdp_address);

#endif	// DRIVERS_ACPI_H