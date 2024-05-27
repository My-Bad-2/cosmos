#include <asm_macros.h>
#include <cpu/registers.h>

.section .text
.extern exception_handler
.extern nmi_handler
interrupt_common:
    // Clear direction flag
    cld

    // Save general purpose registers
    push_reg %r15
    push_reg %r14
    push_reg %r13
    push_reg %r12
    push_reg %r11
    push_reg %r10
    push_reg %r9
    push_reg %r8
    push_reg %rax
    push_reg %rcx
    push_reg %rdx
    push_reg %rbx
    push_reg %rbp
    push_reg %rsi
    push_reg %rdi

    // Zero general-purpose registers to constrain speculative execution with
    // potentially-hostile register values; the values may have come from user
    // mode and could be consumed by in-kernel speculative execution.
    xorl %eax, %eax
    xorl %ebx, %ebx
    xorl %ecx, %ecx
    xorl %edx, %edx
    xorl %esi, %esi
    xorl %edi, %edi
    xorq %r8, %r8
    xorq %r9, %r9
    xorq %r10, %r10
    xorq %r11, %r11
    xorq %r12, %r12
    xorq %r13, %r13
    xorq %r14, %r14
    xorq %r15, %r15

    // Pass the iframe in rdi
    movq %rsp, %rdi

    // Is it Non-maskable interrupt?
    cmpq $2, IFRAME_OFFSET_VECTOR(%rsp)
    je .nmi

    // Is it from user space
    testb $3, IFRAME_OFFSET_CS(%rsp)
    jz 1f

    // Perform the last zero from the previous block
    xorl %ebp, %ebp
    // Swap %gs.base to kernel space
    swapgs
1:
    call exception_handler

    // Check if we're returning to user space as per before
    testb $3, IFRAME_OFFSET_CS(%rsp)
    jz 1f

    // Swap %gs.base to user space
    swapgs
1:

.common_return:
    // Restore general purpose registers.
    pop_reg %rdi
    pop_reg %rsi
    pop_reg %rbp
    pop_reg %rbx
    pop_reg %rdx
    pop_reg %rcx
    pop_reg %rax
    pop_reg %r8
    pop_reg %r9
    pop_reg %r10
    pop_reg %r11
    pop_reg %r12
    pop_reg %r13
    pop_reg %r14
    pop_reg %r15

    // Drop vector number and error code.
    add_to_sp 16

    iretq

.nmi:
    rdmsr64 MSR_IA32_GS_BASE
    mov %rax, %rbx

    .stack_base_from_rsp = INTERRUPT_STACK_SIZE - IFRAME_SIZE
    .percpu_from_rsp = .stack_base_from_rsp + PERCPU_INTERRUPT_STACKS_NMI_OFFSET

    movq %rsp, %rax
    sub $.percpu_from_rsp, %rax

    wrmsr64 MSR_IA32_GS_BASE

    call nmi_handler

    movq %rbx, %rax
    wrmsr64 MSR_IA32_GS_BASE

    JMP_AND_SPECULATION_POSTFENCE(.common_return)

.altmacro
.macro interrupt.define num
int_\num:
    clac
.if !((\num == 8) || ((\num >= 10) && (\num <= 14)) || (\num == 17) || (\num == 21) || (\num == 29) || (\num == 30))
    push_value $0
.endif
    push_value $\num
    JMP_AND_SPECULATION_POSTFENCE(interrupt_common)
.endm

.altmacro
.macro interrupt.declare num
    .quad \num
.endm

.set count, 1
.rept 256
    interrupt.define %count
.set count, count + 1
.endr

.section .data
.global int_table
int_table:
.set count, 0
.rept 256
    interrupt.declare %count
.set count, count + 1
.endr