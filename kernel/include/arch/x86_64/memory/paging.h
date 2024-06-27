#ifndef MEMORY_PAGING_H
#define MEMORY_PAGING_H

#include <kernel.h>
#include <memory/addr.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PTE_FLAG_PRESENT (1 << 0)
#define PTE_FLAG_WRITABLE (1 << 1)
#define PTE_FLAG_USER_ACCESSIBLE (1 << 2)
#define PTE_FLAG_WRITE_THROUGH (1 << 3)
#define PTE_FLAG_NO_CACHE (1 << 4)
#define PTE_FLAG_ACCESSED (1 << 5)
#define PTE_FLAG_DIRTY (1 << 6)
#define PTE_FLAG_HUGE_PAGE (1 << 7)
#define PTE_FLAG_GLOBAL (1 << 8)
#define PTE_FLAG_NO_EXECUTE (1UL << 63)
// Set of flags to be used for all non-terminal page tables (those containing other pagetables, not mappings)
#define PTE_NON_TERMINAL_FLAGS                                                 \
	(PTE_FLAG_PRESENT | PTE_FLAG_WRITABLE | PTE_FLAG_USER_ACCESSIBLE)

#define PTE_ADDRESS_MASK (0x000ffffffffff000)

#define ENTRY_COUNT 512

struct pte {
	uint64_t entry;
};

struct pte_table {
	struct pte entries[ENTRY_COUNT];
} __attribute__((aligned(0x1000)));

extern struct pte_table* kernel_pagemap;

bool pte_is_valid(struct pte* entry);
bool pte_is_large(struct pte* entry);

void pte_set_address(struct pte* entry, phys_addr_t address);
void pte_set_flags(struct pte* entry, size_t flags);

size_t pte_get_flags(struct pte* entry);
phys_addr_t pte_get_address(struct pte* entry);

void* get_next_pml(struct pte_table* curr_lvl, struct pte* entry, bool allocate,
				   virt_addr_t virt_addr, size_t opage_size, size_t page_size);

bool map_page(struct pte_table* toplvl, phys_addr_t phys_addr,
			  virt_addr_t virt_addr, size_t flags, size_t cache, bool lock);
bool unmap_page(struct pte_table* toplvl, virt_addr_t virt_addr, size_t flags,
				bool lock);
bool remap_page(struct pte_table* toplvl, virt_addr_t new_virt_addr,
				virt_addr_t old_virt_addr, size_t flags, size_t cache);
bool setflags_page(struct pte_table* toplvl, virt_addr_t virt_addr,
				   size_t flags, size_t cache, bool lock);

bool map_pages(struct pte_table* toplvl, phys_addr_t phys_addr,
			   virt_addr_t virt_addr, size_t size, size_t flags, size_t cache);
bool unmap_pages(struct pte_table* toplvl, virt_addr_t virt_addr, size_t size,
				 size_t flags);
bool remap_pages(struct pte_table* toplvl, virt_addr_t new_virt_addr,
				 virt_addr_t old_virt_addr, size_t size, size_t flags,
				 size_t cache);
bool setflags_pages(struct pte_table* toplvl, virt_addr_t virt_addr,
					size_t flags, size_t size, size_t cache);

phys_addr_t virt_to_phys_addr(struct pte_table* toplvl, virt_addr_t virt_addr,
							  size_t flags);

void load_paging(struct pte_table* toplvl);
struct pte_table* save_paging(void);

void initialize_pagemap(struct pte_table* toplvl);

#ifdef __cplusplus
}
#endif

#endif	// MEMORY_PAGING_H