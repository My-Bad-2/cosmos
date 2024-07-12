#include "cpu/interrupts.h"
#include "log.h"
#include "uacpi/acpi.h"
#include <drivers/acpi/madt.h>
#include <drivers/ioapic.h>
#include <utils/vector.h>

struct ioapic_isa_override
parse_isa_override(struct acpi_madt_interrupt_source_override* record) {
	if (record->bus != 0) {
		log_fatal("Invalid bus for IOAPIC interrupt override.");
	}

	const uint32_t flags = record->flags;
	enum interrupt_polarity polarity = 0;
	enum interrupt_trigger_mode trigger_mode = 0;

	uint32_t iso_polarity = flags & ACPI_MADT_POLARITY_MASK;
	uint32_t iso_trigger = flags & ACPI_MADT_TRIGGERING_MASK;

	switch (iso_polarity) {
		case ACPI_MADT_POLARITY_CONFORMING:
		case ACPI_MADT_POLARITY_ACTIVE_HIGH:
			polarity = IRQ_POLARITY_ACTIVE_HIGH;
			break;
		case ACPI_MADT_POLARITY_ACTIVE_LOW:
			polarity = IRQ_POLARITY_ACTIVE_LOW;
			break;
		default:
			log_fatal("Unknown IRQ polarity in override: %u", iso_polarity);
	}

	switch (iso_trigger) {
		case ACPI_MADT_TRIGGERING_CONFORMING:
		case ACPI_MADT_TRIGGERING_EDGE:
			trigger_mode = IRQ_TRIGGER_MODE_EDGE;
			break;
		case ACPI_MADT_TRIGGERING_LEVEL:
			trigger_mode = IRQ_TRIGGER_MODE_LEVEL;
			break;
		default:
			log_fatal("Unknown IRQ trigger in override: %u", iso_trigger);
	}

	return (struct ioapic_isa_override){
		.isa_irq = record->source,
		.remapped = true,
		.trigger_mode = trigger_mode,
		.polarity = polarity,
		.global_irq = record->gsi,
	};
}

void arch_drivers_init(void) {
	cvector(struct ioapic_descriptor) descriptors = NULL;
	cvector(struct ioapic_isa_override) isa_overrides = NULL;

	cvector(struct acpi_madt_ioapic) ioapics = get_acpi_ioapic();
	cvector(struct acpi_madt_interrupt_source_override) isos = get_acpi_iso();

	cvector_init(descriptors, cvector_size(ioapics), NULL);
	cvector_init(isa_overrides, cvector_size(isos), NULL);

	for (size_t i = 0; i < cvector_size(ioapics); i++) {
		cvector_vector_type(struct acpi_madt_ioapic) desc =
			cvector_at(ioapics, i);

		struct ioapic_descriptor descriptor = {
			.apic_id = desc->id,
			.global_irq_base = desc->gsi_base,
			.phys_addr = desc->address,
		};

		cvector_push_back(descriptors, descriptor);
	}

	for (size_t i = 0; i < cvector_size(isos); i++) {
		cvector_vector_type(struct acpi_madt_interrupt_source_override) desc =
			cvector_at(isos, i);
		cvector_push_back(isa_overrides, parse_isa_override(desc));
	}

	ioapic_init(descriptors, cvector_size(descriptors), isa_overrides,
				cvector_size(isa_overrides));

	for (uint8_t irq = 0; irq < 8; ++irq) {
		if (irq != 2) {
			ioapic_configure_isa_irq(irq, DELIVERY_MODE_FIXED, IOAPIC_IRQ_MASK,
									 DST_MODE_PHYSICAL, 0, 0);
		}

		ioapic_configure_isa_irq(irq + 8, DELIVERY_MODE_FIXED, IOAPIC_IRQ_MASK,
								 DST_MODE_PHYSICAL, 0, 0);
	}
}