#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/interrupts.h>
#include <cpu/pic.h>
#include <log.h>
#include <memory/heap.h>

#define KERNEL_CODE_SELECTOR (0x8)

extern uint64_t int_table[];
extern void load_idt(struct idt_register* idtr);

void make_idt_segment(struct idt_segment* segment, uint64_t handler,
					  uint8_t ist, uint8_t attribute, uint16_t selector) {
	segment->offset_low = handler & 0xffff;
	segment->selector = selector;
	segment->ist = ist;
	segment->type_attribute = attribute;
	segment->offset_mid = (handler >> 16) & 0xffff;
	segment->offset_high = (handler >> 32) & 0xffffffff;
	segment->reserved = 0;
}

void idt_init(void) {
	struct idt_table* idt = heap_malloc(sizeof(struct idt_table));

	for (int i = 0; i < IDT_MAX_ENTRY; ++i) {
		make_idt_segment(&idt->entries[i], int_table[i], 0, IDT_INTERRUPT_GATE,
						 KERNEL_CODE_SELECTOR);
	}

	struct idt_register idtr = {
		(sizeof(struct idt_table) - 1),
		(uintptr_t)idt,
	};

	load_idt(&idtr);

	interrupt_handler_init();
	pic_remap(PLATFORM_INTERRUPT_BASE, PLATFORM_INTERRUPT_BASE + 8);

	log_info("Initialized Interrupt Descriptor Table!");
}