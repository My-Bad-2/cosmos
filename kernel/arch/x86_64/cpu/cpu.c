#include <arch.h>
#include <cpu/cpu.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/registers.h>

bool cpuid(cpuid_leaf_t* cpuid_leaf, uint32_t leaf, uint32_t subleaf) {
	uint32_t cpuid_max = 0;

	asm volatile("cpuid"
				 : "=a"(cpuid_max)
				 : "a"(leaf & 0x80000000)
				 : "ebx", "ecx", "edx");

	if (leaf > cpuid_max) {
		return false;
	}

	asm volatile("cpuid"
				 : "=a"(cpuid_leaf->eax), "=b"(cpuid_leaf->ebx),
				   "=c"(cpuid_leaf->ecx), "=d"(cpuid_leaf->edx)
				 : "a"(leaf), "c"(subleaf));

	return true;
}

uint64_t read_msr(uint32_t msr) {
	uint32_t edx, eax;
	asm volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(msr) : "memory");
	return ((uint64_t)(edx) << 32) | eax;
}

void write_msr(uint32_t msr, uint64_t value) {
	uint32_t edx = value >> 32;
	uint32_t eax = (uint32_t)(value);
	asm volatile("wrmsr" ::"a"(eax), "d"(edx), "c"(msr) : "memory");
}

void write_cr0(uint64_t val) {
	asm volatile("mov %0, %%cr0" ::"r"(val) : "memory");
}

void write_cr3(uint64_t val) {
	asm volatile("mov %0, %%cr3" ::"r"(val) : "memory");
}

void write_cr4(uint64_t val) {
	asm volatile("mov %0, %%cr4" ::"r"(val) : "memory");
}

uint64_t read_cr0(void) {
	uint64_t val = 0;
	asm volatile("mov %%cr0, %0" : "=r"(val)::"memory");
	return val;
}

uint64_t read_cr3(void) {
	uint64_t val = 0;
	asm volatile("mov %%cr3, %0" : "=r"(val)::"memory");
	return val;
}

uint64_t read_cr4(void) {
	uint64_t val = 0;
	asm volatile("mov %%cr4, %0" : "=r"(val)::"memory");
	return val;
}

void invlpg(uint64_t address) {
	asm volatile("invlpg (%0)" ::"r"(address));
}

void cpu_enable_pat(void) {
	write_msr(MSR_IA32_PAT, PAT_CUSTOM_STATE);
}

void cpu_init() {
	arch_disable_interrupts();

	gdt_init();
	idt_init();

	arch_enable_interrupts();
}