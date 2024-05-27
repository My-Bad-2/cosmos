#include <cpu/gdt.h>
#include <stdint.h>
#include <log.h>

#define gdt_null_segment(segment) create_gdt_segment(segment, 0, 0, 0, 0)
#define gdt_segment(segment, granularity, access)                              \
	create_gdt_segment(segment, 0, 0xFFFFFFFF, granularity, access)

gdt_t global_gdt = {};
tss_t tss = {};
gdt_register_t gdtr = {
    sizeof(gdt_t) - 1,
    (uint64_t)&global_gdt,
};

extern void gdt_load(gdt_register_t* gdtr);
extern void tss_load(void);

static void create_gdt_segment(gdt_segment_t* segment, uint32_t base,
							   uint32_t limit, uint8_t granularity,
							   uint8_t access) {
	segment->limit_low = (uint16_t)(limit & 0xFFFF);
	segment->base_low = (uint16_t)(base & 0xFFFF);
	segment->base_mid = (uint8_t)((base >> 16) & 0xFF);
	segment->access = access;
	segment->limit_high = (limit >> 16) & 0x0F;
	segment->granularity = granularity;
	segment->base_high = (uint8_t)((base >> 24) & 0xFF);
}

static void create_tss_segment(tss_segment_t* segment, tss_t* tss) {
	segment->len = sizeof(tss_t);
	segment->base_low = (uint16_t)((uintptr_t)tss & 0xFFFF);
	segment->base_mid = (uint8_t)(((uintptr_t)tss >> 16) & 0xFF);
	segment->flags_low = 0x89;
	segment->flags_high = 0;
	segment->base_high = (uint8_t)(((uintptr_t)tss >> 24) & 0xFF);
	segment->base_upper = (uint32_t)(((uintptr_t)tss >> 32));
	segment->reserved = 0;
}

void gdt_init() {
	gdt_null_segment(&global_gdt.null);

	gdt_segment(&global_gdt.kernel_code, GDT_FLAGS_LONG_MODE,
				GDT_FLAGS_PRESENT | GDT_FLAGS_SYSTEM | GDT_FLAGS_EXECUTABLE |
					GDT_FLAGS_RW);

	gdt_segment(&global_gdt.kernel_data, 0,
				GDT_FLAGS_PRESENT | GDT_FLAGS_SYSTEM | GDT_FLAGS_RW);

	gdt_segment(&global_gdt.user_data, 0,
				GDT_FLAGS_PRESENT | GDT_FLAGS_DPL3 | GDT_FLAGS_SYSTEM |
					GDT_FLAGS_RW);

	gdt_segment(&global_gdt.user_code, 0,
				GDT_FLAGS_PRESENT | GDT_FLAGS_DPL3 | GDT_FLAGS_SYSTEM |
					GDT_FLAGS_EXECUTABLE | GDT_FLAGS_RW);

    create_tss_segment(&global_gdt.tss, &tss);

    gdt_load(&gdtr);
    tss_load();

    log_info("Successfully loaded GDT.");
}