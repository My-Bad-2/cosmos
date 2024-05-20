#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define TSS_SELECTOR 0x28

.section .text

.global gdt_load
gdt_load:
    lgdtq (%rdi)
    movw $KERNEL_DATA_SELECTOR, %ax
    movl %eax, %ds
    movl %eax, %fs
    movl %eax, %gs
    movl %eax, %es
    movl %eax, %ss
    pushq $KERNEL_CODE_SELECTOR
    lea .gdt_trampoline(%rip), %rax
    pushq %rax
    lretq
.gdt_trampoline:
    retq

.global tss_load
tss_load:
    movw $TSS_SELECTOR, %ax
    ltrw %ax
    retq
