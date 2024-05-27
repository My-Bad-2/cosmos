#include <cpu/gdt.h>
#include <cpu/cpu.h>

void cpu_init() {
    gdt_init();
}