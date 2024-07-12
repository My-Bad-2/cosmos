#ifndef DRIVERS_IOAPIC_H
#define DRIVERS_IOAPIC_H

#include <cpu/interrupts.h>
#include <memory/addr.h>

#define INVALID_APIC_ID 0xffffffff
#define APIC_PHYS_BASE 0xfee00000
#define APIC_BASE_BSP (1u << 8)
#define APIC_BASE_X2APIC_ENABLE (1u << 10)
#define APIC_BASE_XAPIC_ENABLE (1u << 11)
#define NUM_ISA_IRQS 16

#define IOAPIC_IOREGSEL 0x00
#define IOAPIC_IOWIN 0x10

#define IOAPIC_REG_ID 0x00
#define IOAPIC_REG_VER 0x01
#define IOAPIC_IRQ_MASK true
#define IOAPIC_IRQ_UNMASK false

enum apic_interrupt_delivery_mode {
	DELIVERY_MODE_FIXED = 0,
	DELIVERY_MODE_LOWEST_PRI = 1,
	DELIVERY_MODE_SMI = 2,
	DELIVERY_MODE_NMI = 4,
	DELIVERY_MODE_INIT = 5,
	DELIVERY_MODE_STARTUP = 6,
	DELIVERY_MODE_EXT_INT = 7,
};

enum apic_interrupt_dst_mode {
	DST_MODE_PHYSICAL = 0,
	DST_MODE_LOGICAL = 1,
};

// Global system interrupts in the range [start, end).
struct gsi_range {
	uint32_t start;
	uint32_t end;
};

struct ioapic_descriptor {
	uint8_t apic_id;
	uint32_t global_irq_base;
	phys_addr_t phys_addr;
};

struct ioapic_isa_override {
	uint8_t isa_irq;
	bool remapped;
	enum interrupt_trigger_mode trigger_mode;
	enum interrupt_polarity polarity;
	uint32_t global_irq;
};

void ioapic_init(struct ioapic_descriptor* ioapic_descs, size_t ioapics_num,
				 struct ioapic_isa_override* overrides, size_t num_overrides);

bool ioapic_is_enabled(void);

bool ioapic_is_valid_irq(uint32_t global_irq);
void ioapic_set_irq_mask(uint32_t global_irq);
void ioapic_clear_irq_mask(uint32_t global_irq);

void ioapic_configure_irq(uint32_t global_irq,
						  enum interrupt_trigger_mode trig_mode,
						  enum interrupt_polarity polarity,
						  enum apic_interrupt_delivery_mode del_mode, bool mask,
						  enum apic_interrupt_dst_mode dst_mode, uint8_t dst,
						  uint8_t vector);

void ioapic_fetch_irq_config(uint32_t global_irq,
							 enum interrupt_trigger_mode* trig_mode,
							 enum interrupt_polarity* polarity);
void ioapic_configure_irq_vector(uint32_t global_irq, uint8_t vector);
uint8_t ioapic_fetch_irq_vector(uint32_t global_irq);

void ioapic_set_mask_isa_irq(uint8_t isa_irq);
void ioapic_clear_mask_isa_irq(uint8_t isa_irq);

void ioapic_configure_isa_irq(uint8_t isa_irq,
							  enum apic_interrupt_delivery_mode del_mode,
							  bool mask, enum apic_interrupt_dst_mode dst_mode,
							  uint8_t dst, uint8_t vector);
void ioapic_issue_eoi(uint32_t global_irq, uint8_t vec);
uint32_t ioapic_isa_to_global(uint8_t isa_irq);

// Returns the [min, max) range representing the (assumed) contiguous range of
// global system interrupts provided to us by ACPI.
struct gsi_range ioapic_get_gsi_range();

void ioapic_save();
void ioapic_restore();

#endif	// DRIVERS_IOAPIC_H