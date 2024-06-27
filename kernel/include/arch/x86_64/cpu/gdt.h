#ifndef CPU_GDT_H
#define CPU_GDT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GDT_FLAGS_PRESENT (1 << 7)
#define GDT_FLAGS_DPL3 (3 << 5)
#define GDT_FLAGS_SYSTEM (1 << 4)
#define GDT_FLAGS_EXECUTABLE (1 << 3)
#define GDT_FLAGS_DC (1 << 2)
#define GDT_FLAGS_RW (1 << 1)
#define GDT_FLAGS_ACCESS (1 << 0)

#define GDT_FLAGS_GRANULARITY (1 << 3)
#define GDT_FLAGS_DB (1 << 2)
#define GDT_FLAGS_LONG_MODE (1 << 1)

struct tss {
	uint32_t reserved1;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved2;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t reserved3;
	uint16_t reserved4;
	uint16_t iopb;
} __attribute__((packed));

struct gdt_segment {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t limit_high : 4;
	uint8_t granularity : 4;
	uint8_t base_high;
} __attribute__((packed));

struct tss_segment {
	uint16_t len;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t flags_low;
	uint8_t flags_high;
	uint8_t base_high;
	uint32_t base_upper;
	uint32_t reserved;
} __attribute__((packed));

struct gdt_table {
	struct gdt_segment null;
	struct gdt_segment kernel_code;
	struct gdt_segment kernel_data;
	struct gdt_segment user_data;
	struct gdt_segment user_code;
	struct tss_segment tss;
} __attribute__((packed));

struct gdt_register {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

void gdt_init(void);

#ifdef __cplusplus
}
#endif

#endif	// CPU_GDT_H