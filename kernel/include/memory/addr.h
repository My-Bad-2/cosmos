#ifndef MEMORY_ADDR_H
#define MEMORY_ADDR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define INVALID_ADDR ((uintptr_t)(-1))

#ifdef __cplusplus
extern "C" {
#endif

typedef uintptr_t phys_addr_t;
typedef uintptr_t virt_addr_t;

virt_addr_t to_higher_half(phys_addr_t address);
phys_addr_t from_higher_half(virt_addr_t address);
bool is_higher_half(uintptr_t address);

uintptr_t align_down(uintptr_t address, size_t page_size);
uintptr_t align_up(uintptr_t address, size_t page_size);
bool is_aligned(uintptr_t address, size_t page_size);

#ifdef __cplusplus
}
#endif

#endif	// MEMORY_ADDR_H