#ifndef CPU_REGISTERS_H
#define CPU_REGISTERS_H

#define INTERRUPT_STACK_SIZE (4096)
#define PERCPU_INTERRUPT_STACKS_NMI_OFFSET 0x20E0

#define IFRAME_OFFSET_RDI (0 * 8)
#define IFRAME_OFFSET_RSI (1 * 8)
#define IFRAME_OFFSET_RBP (2 * 8)
#define IFRAME_OFFSET_RBX (3 * 8)
#define IFRAME_OFFSET_RDX (4 * 8)
#define IFRAME_OFFSET_RCX (5 * 8)
#define IFRAME_OFFSET_RAX (6 * 8)
#define IFRAME_OFFSET_R8 (7 * 8)
#define IFRAME_OFFSET_R9 (8 * 8)
#define IFRAME_OFFSET_R10 (9 * 8)
#define IFRAME_OFFSET_R11 (10 * 8)
#define IFRAME_OFFSET_R12 (11 * 8)
#define IFRAME_OFFSET_R13 (12 * 8)
#define IFRAME_OFFSET_R14 (13 * 8)
#define IFRAME_OFFSET_R15 (14 * 8)

#define IFRAME_OFFSET_VECTOR (15 * 8)
#define IFRAME_OFFSET_ERR_CODE (16 * 8)

#define IFRAME_OFFSET_IP (17 * 8)
#define IFRAME_OFFSET_CS (18 * 8)
#define IFRAME_OFFSET_FLAGS (19 * 8)
#define IFRAME_OFFSET_USER_SP (20 * 8)
#define IFRAME_OFFSET_USER_SS (21 * 8)

#define IFRAME_SIZE (22 * 8)

#define MSR_IA32_EFER 0xc0000080
#define MSR_IA32_STAR 0xc0000081
#define MSR_IA32_LSTAR 0xc0000082
#define MSR_IA32_CSTAR 0xc0000083
#define MSR_IA32_FMASK 0xc0000084
#define MSR_IA32_FS_BASE 0xc0000100
#define MSR_IA32_GS_BASE 0xc0000101
#define MSR_IA32_KERNEL_GS_BASE 0xc0000102
#define MSR_IA32_TSC_AUX 0xc0000103
#define MSR_IA32_PM_ENABLE 0x00000770
#define MSR_IA32_HWP_CAPABILITIES 0x00000771
#define MSR_IA32_HWP_REQUEST 0x00000774
#define MSR_AMD_VIRT_SPEC_CTRL 0xc001011f

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct {
	uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	uint64_t vector;
	uint64_t err_code;
	uint64_t ip, cs, flags;
	uint64_t user_sp, user_ss;
} iframe_t;

#endif	// __ASSEMBLER__

#endif	// CPU_REGISTERS_H