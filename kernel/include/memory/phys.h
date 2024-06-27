#ifndef MEMORY_PHYS_H
#define MEMORY_PHYS_H

#include <memory/addr.h>

#ifdef __cplusplus
extern "C" {
#endif

phys_addr_t alloc_phys_page(void);
phys_addr_t alloc_phys_pages(size_t count);

phys_addr_t highest_phys_addr(void);

void free_phys_page(void* address);
void free_phys_pages(void* address, size_t count);

void lock_phys_page(void);
void lock_phys_pages(size_t count);

void phys_init(void);
void dump_phys_info(void);

#ifdef __cplusplus
}
#endif

#endif	// MEMORY_PHYS_H