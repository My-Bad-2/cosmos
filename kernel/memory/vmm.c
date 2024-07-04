#include <assert.h>
#include <log.h>
#include <memory/heap.h>
#include <memory/memory.h>
#include <memory/paging.h>
#include <memory/phys.h>
#include <memory/vmm.h>

#define is_power_of_two(n) (((n) & ((n) - 1)) == 0)

struct pte_table* kernel_pagemap = NULL;

void* get_next_pml(struct pte_table* curr_lvl, struct pte* entry, bool allocate,
				   virt_addr_t virt_addr, size_t opage_size, size_t page_size) {
	void* ret = NULL;

	if (pte_is_valid(entry)) {
		if (pte_is_large(entry) && opage_size != (size_t)(-1)) {
			struct vm_flags vm_flags = parse_vm_flags(
				pte_get_flags(entry), opage_size > PAGE_SIZE_4KiB);
			phys_addr_t old_phys_addr = pte_get_address(entry);
			virt_addr_t old_virt_addr = virt_addr & ~(opage_size - 1);

			if (old_phys_addr & (opage_size - 1)) {
				log_fatal("Unexpected page table entry address: %#08lx",
						  old_phys_addr);
			}

			ret = (void*)to_higher_half(alloc_phys_page());
			pte_set_address(entry, from_higher_half((uintptr_t)ret));
			pte_set_flags(entry, PTE_NON_TERMINAL_FLAGS);

			for (size_t i = 0; i < opage_size; i += page_size) {
				map_page(curr_lvl, old_phys_addr + 1, old_virt_addr + 1,
						 vm_flags.flags | page_size_flags(page_size),
						 vm_flags.cache, false);
			}
		} else {
			ret = (void*)to_higher_half(pte_get_address(entry));
		}
	} else if (allocate) {
		ret = (void*)to_higher_half(alloc_phys_page());
		pte_set_address(entry, from_higher_half((uintptr_t)ret));
		pte_set_flags(entry, PTE_NON_TERMINAL_FLAGS);
	}

	return ret;
}

void vmm_init(void) {
	struct limine_memmap_entry** memmaps = memmap_request.response->entries;
	size_t memmap_count = memmap_request.response->entry_count;

	vm_init();

	kernel_pagemap = (void*)to_higher_half(alloc_phys_page());
	initialize_pagemap(kernel_pagemap);

	size_t psize = PAGE_SIZE_1GiB;
	size_t flags = page_size_flags(psize);

	for (phys_addr_t i = 0; i < (PAGE_SIZE_1GiB * 4); i += psize) {
		assert(map_page(kernel_pagemap, i, to_higher_half(i),
						VMM_FLAG_RW | flags, 0, true));
	}

	for (size_t i = 0; i < memmap_count; ++i) {
		struct limine_memmap_entry* entry = memmaps[i];

		phys_addr_t base = align_down(entry->base, PAGE_SIZE);
		phys_addr_t top = align_up(entry->base + entry->length, PAGE_SIZE);

		if (top < PAGE_SIZE_1GiB * 4) {
			continue;
		}

		size_t cache = VMM_CACHE_WRITE_BACK;

		if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER) {
			cache = VMM_CACHE_FRAMEBUFFER;
		}

		size_t size = top - base;
		size_t page_size = get_required_size(size);
		size_t page_flag = page_size_flags(page_size);

		size_t aligned_size = align_down(size, page_size);
		phys_addr_t diff = size - aligned_size;

		for (phys_addr_t j = base; j < (base + aligned_size); j += page_size) {
			if (j < (PAGE_SIZE_1GiB * 4)) {
				continue;
			}

			assert(map_page(kernel_pagemap, j, to_higher_half(j),
							VMM_FLAG_RW | page_flag, cache, true));
		}

		base += aligned_size;

		for (phys_addr_t j = base; j < (base + diff); j += PAGE_SIZE) {
			if (j < (PAGE_SIZE_1GiB * 4)) {
				continue;
			}

			assert(map_page(kernel_pagemap, j, to_higher_half(j), VMM_FLAG_RW,
							cache, true));
		}
	}

	for (size_t i = 0; i < kernel_file_request.response->kernel_file->size;
		 i += PAGE_SIZE) {
		phys_addr_t phys_addr =
			kernel_address_request.response->physical_base + i;
		virt_addr_t virt_addr =
			kernel_address_request.response->virtual_base + i;

		assert(map_page(kernel_pagemap, phys_addr, virt_addr, VMM_FLAG_RWX,
						VMM_CACHE_WRITE_BACK, true));
	}

	load_paging(kernel_pagemap);

	log_info("Initialized Virtual Memory Manager!");
}

// Temp fix until virtual memory allocator is implemented
void* virt_alloc(size_t size, size_t alignment) {
	assert(is_power_of_two(alignment));

	size_t alloc_size = alignment ? align_up(size, alignment) : size;
	return heap_malloc(alloc_size);
}

void virt_free(void* ptr) {
	heap_free(ptr);
}