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

typedef struct {
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
} __attribute__((packed)) tss_t;

typedef struct {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t limit_high : 4;
	uint8_t granularity : 4;
	uint8_t base_high;
} __attribute__((packed)) gdt_segment_t;

typedef struct {
	uint16_t len;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t flags_low;
	uint8_t flags_high;
	uint8_t base_high;
	uint32_t base_upper;
	uint32_t reserved;
} __attribute__((packed)) tss_segment_t;

typedef struct {
	gdt_segment_t null;
	gdt_segment_t kernel_code;
	gdt_segment_t kernel_data;
	gdt_segment_t user_data;
	gdt_segment_t user_code;
	tss_segment_t tss;
} __attribute__((packed)) gdt_t;

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) gdt_register_t;

void gdt_init(void);

#ifdef __cplusplus
}
#endif

#endif	// CPU_GDT_H