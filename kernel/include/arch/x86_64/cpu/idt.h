#ifndef CPU_IDT_H
#define CPU_IDT_H

#include <stdint.h>

#define IDT_INTERRUPT_GATE (0x8e)
#define IDT_TRAP_GATE (0xef)
#define IDT_DPL3 (0x60)

#define IDT_MAX_ENTRY (256)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t ist;
	uint8_t type_attribute;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t reserved;
} __attribute__((packed)) idt_segment_t;

typedef struct {
	idt_segment_t entries[IDT_MAX_ENTRY];
} __attribute__((packed)) idt_t;

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) idt_register_t;

void idt_init(void);

#ifdef __cplusplus
}
#endif

#endif	// CPU_IDT_H