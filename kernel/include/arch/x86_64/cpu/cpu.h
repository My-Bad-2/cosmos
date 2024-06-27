#ifndef CPU_CPU_H
#define CPU_CPU_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PAT_UNCACHABLE_STRONG 0ULL
#define PAT_WRITE_COMBINING 1ULL
#define PAT_WRITE_THROUGH 4ULL
#define PAT_WRITE_PROTECTED 5ULL
#define PAT_WRITE_BACK 6ULL
#define PAT_UNCACHABLE 7ULL

#define PAT_RESET_STATE (0x0007040600070406ULL)
#define PAT_CUSTOM_STATE                                                       \
	((PAT_UNCACHABLE << 56) | (PAT_WRITE_BACK << 48) |                         \
	 (PAT_WRITE_PROTECTED << 40) | (PAT_WRITE_THROUGH << 32) |                 \
	 (PAT_WRITE_COMBINING << 24) | (PAT_UNCACHABLE_STRONG << 16))

#define CPUID_LEAF_INITIALIZER ((struct cpuid_leaf){0, 0, 0, 0})

struct cpuid_leaf {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
};

bool cpuid(struct cpuid_leaf* cpuid_leaf, uint32_t leaf, uint32_t subleaf);

uint64_t read_msr(uint32_t msr);
void write_msr(uint32_t msr, uint64_t val);

void write_cr0(uint64_t val);
void write_cr3(uint64_t val);
void write_cr4(uint64_t val);

uint64_t read_cr0(void);
uint64_t read_cr2(void);
uint64_t read_cr3(void);
uint64_t read_cr4(void);

void invlpg(void* address);

void cpu_init(void);
void cpu_enable_pat(void);

#ifdef __cplusplus
}
#endif

#endif	// CPU_CPU_H