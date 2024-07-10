#include "uacpi/acpi.h"
#include <drivers/acpi/madt.h>
#include <log.h>
#include <memory/addr.h>
#include <uacpi/tables.h>
#include <utils/vector.h>

struct acpi_madt* madt_table = NULL;
cvector_vector_type(struct acpi_madt_ioapic) madt_ioapics = NULL;
cvector_vector_type(struct acpi_madt_interrupt_source_override) madt_isos =
	NULL;

struct acpi_madt_ioapic* get_acpi_ioapic(void) {
	return madt_ioapics;
}

struct acpi_madt_interrupt_source_override* get_acpi_iso(void) {
	return madt_isos;
}

void madt_init(void) {
	cvector_init(madt_ioapics, 1, NULL);
	cvector_init(madt_isos, 1, NULL);

	uacpi_table out_table;

	if (uacpi_table_find_by_signature(ACPI_MADT_SIGNATURE, &out_table) !=
		UACPI_STATUS_OK) {
		return;
	}

	madt_table = (struct acpi_madt*)out_table.virt_addr;

	virt_addr_t madt_start = (virt_addr_t)(madt_table->entries);
	virt_addr_t madt_end = (virt_addr_t)madt_table + madt_table->hdr.length;

	struct acpi_entry_hdr* madt_hdr = (struct acpi_entry_hdr*)madt_start;

	for (virt_addr_t entry = madt_start; entry < madt_end;
		 entry += madt_hdr->length, madt_hdr = (struct acpi_entry_hdr*)entry) {
		switch (madt_hdr->type) {
			case ACPI_MADT_ENTRY_TYPE_IOAPIC:
				cvector_push_back(madt_ioapics,
								  *(struct acpi_madt_ioapic*)entry);
				break;
			case ACPI_MADT_ENTRY_TYPE_INTERRUPT_SOURCE_OVERRIDE:
				cvector_push_back(
					madt_isos,
					*(struct acpi_madt_interrupt_source_override*)entry);
				break;
		}
	}

	cvector_shrink_to_fit(madt_ioapics);
	cvector_shrink_to_fit(madt_isos);
}