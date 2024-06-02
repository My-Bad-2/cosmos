#include <cpu/gdt.h>
#include <cpu/idt.h>

#include <log.h>

#define KERNEL_CODE_SELECTOR (0x8)

extern uint64_t int_table[];
extern void load_idt(idt_register_t* idtr);

idt_t global_idt = {};
idt_register_t idtr = {
	(sizeof(idt_t) - 1),
	(uint64_t)&global_idt,
};

static void make_idt_segment(idt_segment_t* segment, uint64_t handler,
							 uint8_t ist, uint8_t attribute,
							 uint16_t selector) {
	segment->offset_low = handler & 0xffff;
	segment->selector = selector;
	segment->ist = ist;
	segment->type_attribute = attribute;
	segment->offset_mid = (handler >> 16) & 0xffff;
	segment->offset_high = (handler >> 32) & 0xffffffff;
	segment->reserved = 0;
}

void idt_init(void) {
	for (int i = 0; i < IDT_MAX_ENTRY; ++i) {
		make_idt_segment(&global_idt.entries[i], int_table[i], 0,
						 IDT_INTERRUPT_GATE, KERNEL_CODE_SELECTOR);
	}

	load_idt(&idtr);

	log_info("Successfully initialized Interrupt Descriptor Table.");
}