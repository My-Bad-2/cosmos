#ifndef ARCH_ASM_MACROS_H
#define ARCH_ASM_MACROS_H

// clang-format off

// Use these when pushing a register from the calling frame.

.macro push_reg reg
pushq \reg
.endm

.macro pop_reg reg
popq \reg
.endm

// Use these when pushing a value other than the above, or when pushing
// a register that is not from the calling frame.

.macro push_value value
pushq \value
.endm

.macro pop_value value
popq \value
.endm

// Use these when adding/subtracting values from the stack pointer.

.macro sub_from_sp value
sub $\value, %rsp
.endm

.macro add_to_sp value
add $\value, %rsp
.endm

#define RET_AND_SPECULATION_POSTFENCE \
    ret; \
    int3

#define JMP_AND_SPECULATION_POSTFENCE(x) \
    jmp x; \
    int3

// Writes %rax to the given MSR, which should be the bare constant.
// Clobbers %rcx and %rdx.
.macro wrmsr64 msr
    mov $\msr, %ecx
    mov %rax, %rdx
    shr $32, %rdx
    wrmsr
.endm

// Reads the given MSR, which should be the bare constant, into %rax.
// Clobbers %rcx and %rdx.
.macro rdmsr64 msr
    mov $\msr, %ecx
    rdmsr
    shl $32, %rdx
    or %rdx, %rax
.endm


#endif // ARCH_ASM_MACROS_H