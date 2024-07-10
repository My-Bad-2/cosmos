#ifndef DRIVERS_ACPI_MADT_H
#define DRIVERS_ACPI_MADT_H

#include <uacpi/acpi.h>

struct acpi_madt_ioapic* get_acpi_ioapic(void);
struct acpi_madt_interrupt_source_override* get_acpi_iso(void);
void madt_init(void);

#endif	// DRIVERS_ACPI_MADT_H