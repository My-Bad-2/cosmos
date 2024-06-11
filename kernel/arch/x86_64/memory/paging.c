#include <log.h>

#include <cpu/cpu.h>
#include <memory/memory.h>
#include <memory/paging.h>
#include <memory/vmm.h>
#include <utils/sync.h>

static lock_t vm_lock = LOCK_INITIALIZER;
static bool pml3_translation = false;

void pte_set_flags(pte_t* entry, size_t flags) {
	uint64_t temp = entry->entry;
	temp &= ~flags;
	temp |= flags;
	entry->entry = temp;
}

phys_addr_t pte_get_address(pte_t* entry) {
	return entry->entry & PTE_ADDRESS_MASK;
}

size_t pte_get_flags(pte_t* entry) {
	return entry->entry & ~PTE_ADDRESS_MASK;
}

void pte_set_address(pte_t* entry, phys_addr_t address) {
	uint64_t temp = entry->entry;
	temp &= ~PTE_ADDRESS_MASK;
	temp |= address;
	entry->entry = temp;
}

bool pte_is_valid(pte_t* entry) {
	return pte_get_flags(entry) & PTE_FLAG_PRESENT;
}

bool pte_is_large(pte_t* entry) {
	return pte_get_flags(entry) & PTE_FLAG_HUGE_PAGE;
}

static size_t get_page_size(size_t flags) {
	size_t page_size = PAGE_SIZE_4KiB;

	if (flags & VMM_FLAG_LARGE_PAGE) {
		page_size = PAGE_SIZE_2MiB;
	}

	if (flags & VMM_FLAG_HUGE_PAGE) {
		page_size = PAGE_SIZE_1GiB;
	}

	return page_size;
}

static size_t parse_cache(size_t cache, bool large_pages) {
	uint64_t patbit = (large_pages ? (1 << 12) : (1 << 7));
	size_t ret = 0;

	switch (cache) {
		case VMM_CACHE_UNCACHEABLE_STRONG:
			ret |= PTE_FLAG_NO_CACHE;
			break;
		case VMM_CACHE_WRITE_COMBINING:
			ret |= PTE_FLAG_NO_CACHE | PTE_FLAG_WRITE_THROUGH;
			break;
		case VMM_CACHE_WRITE_THROUGH:
			ret |= patbit;
			break;
		case VMM_CACHE_WRITE_PROTECTED:
			ret |= patbit | PTE_FLAG_WRITE_THROUGH;
			break;
		case VMM_CACHE_WRITE_BACK:
			ret |= patbit | PTE_FLAG_NO_CACHE;
			break;
		case VMM_CACHE_UNCACHEABLE:
			ret |= patbit | PTE_FLAG_NO_CACHE | PTE_FLAG_WRITE_THROUGH;
			break;
	}

	return ret;
}

static size_t parse_vmm_flags(size_t flags) {
	size_t ret = PTE_FLAG_PRESENT;

	if (flags & VMM_FLAG_WRITE) {
		ret |= PTE_FLAG_WRITABLE;
	}

	if (flags & VMM_FLAG_USER) {
		ret |= PTE_FLAG_USER_ACCESSIBLE;
	}

	if (flags & VMM_FLAG_GLOBAL) {
		ret |= PTE_FLAG_GLOBAL;
	}

	if ((flags & VMM_FLAG_LARGE_PAGE) || (flags & VMM_FLAG_HUGE_PAGE)) {
		ret |= PTE_FLAG_HUGE_PAGE;
	}

	if (!(flags & VMM_FLAG_EXECUTE)) {
		ret |= PTE_FLAG_NO_EXECUTE;
	}

	return ret;
}

static pte_t* virt_to_pte(pte_table_t* curr_lvl, virt_addr_t virt_addr,
						  bool allocate, size_t page_size, bool large_pages) {
	size_t pml5_entry = (virt_addr & (0x1FFUL << 48)) >> 48;
	size_t pml4_entry = (virt_addr & (0x1FFUL << 39)) >> 39;
	size_t pml3_entry = (virt_addr & (0x1FFUL << 30)) >> 30;
	size_t pml2_entry = (virt_addr & (0x1FFUL << 21)) >> 21;
	size_t pml1_entry = (virt_addr & (0x1FFUL << 12)) >> 12;

	pte_table_t *pml4, *pml3, *pml2, *pml1;

	if (curr_lvl == NULL) {
		return NULL;
	}

	if (paging_mode_request.response->mode == LIMINE_PAGING_MODE_MAX) {
		pml4 = (pte_table_t*)get_next_pml(
			curr_lvl, &curr_lvl->entries[pml5_entry], allocate, -1, -1, -1);
	} else {
		pml4 = curr_lvl;
	}

	if (curr_lvl == NULL) {
		return NULL;
	}

	pml3 = (pte_table_t*)get_next_pml(curr_lvl, &pml4->entries[pml4_entry],
									  allocate, -1, -1, -1);

	if (pml3 == NULL) {
		return NULL;
	}

	if (page_size == PAGE_SIZE_1GiB ||
		(large_pages && pte_is_large(&pml3->entries[pml3_entry]))) {
		return &pml3->entries[pml3_entry];
	}

	pml2 = (pte_table_t*)get_next_pml(curr_lvl, &pml3->entries[pml3_entry],
									  allocate, virt_addr, PAGE_SIZE_1GiB,
									  page_size);

	if (pml2 == NULL) {
		return NULL;
	}

	if (page_size == PAGE_SIZE_2MiB ||
		(large_pages && pte_is_large(&pml2->entries[pml2_entry]))) {
		return &pml2->entries[pml2_entry];
	}

	pml1 = (pte_table_t*)get_next_pml(curr_lvl, &pml2->entries[pml2_entry],
									  allocate, virt_addr, PAGE_SIZE_2MiB,
									  page_size);

	return &pml1->entries[pml1_entry];
}

phys_addr_t virt_to_phys_addr(pte_table_t* toplvl, virt_addr_t virt_addr,
							  size_t flags) {
	try_lock(&vm_lock);

	size_t page_size = get_page_size(flags);
	pte_t* pml_entry = virt_to_pte(toplvl, virt_addr, false, page_size, true);

	if (pml_entry == NULL || !(pte_get_flags(pml_entry) & PTE_FLAG_PRESENT)) {
		return INVALID_ADDR;
	}

	lock_release(&vm_lock);

	return pte_get_address(pml_entry) + (virt_addr % page_size);
}

static bool map_internal(pte_table_t* toplvl, phys_addr_t phys_addr,
						 virt_addr_t virt_addr, size_t flags, size_t cache,
						 size_t page_size) {
	pte_t* pml_entry = virt_to_pte(toplvl, virt_addr, true, page_size, true);

	if (pml_entry == NULL) {
		log_error("Could not get pagemap entry for address: %#08lx", virt_addr);
		return false;
	}

	size_t real_flags =
		parse_vmm_flags(flags) | parse_cache(cache, page_size > PAGE_SIZE_4KiB);

	pml_entry->entry = 0;
	pte_set_address(pml_entry, phys_addr);
	pte_set_flags(pml_entry, real_flags);

	return true;
}

bool map_page(pte_table_t* toplvl, phys_addr_t phys_addr, virt_addr_t virt_addr,
			  size_t flags, size_t cache, bool lock) {
	if (lock) {
		try_lock(&vm_lock);
	}

	size_t page_size = get_page_size(flags);

	if (page_size == PAGE_SIZE_1GiB && !pml3_translation) {
		flags &= ~VMM_FLAG_HUGE_PAGE;
		flags |= VMM_FLAG_LARGE_PAGE;

		for (size_t i = 0; i < PAGE_SIZE_1GiB; i += PAGE_SIZE_2MiB) {
			if (!map_internal(toplvl, phys_addr + i, virt_addr + i, flags,
							  cache, PAGE_SIZE_2MiB)) {
				return false;
			}
		}

		if (lock) {
			lock_release(&vm_lock);
		}

		return true;
	}

	if (lock) {
		lock_release(&vm_lock);
	}

	return map_internal(toplvl, phys_addr, virt_addr, flags, cache, page_size);
}

static bool unmap_internal(pte_table_t* toplvl, virt_addr_t virt_addr,
						   size_t page_size) {
	pte_t* pml_entry = virt_to_pte(toplvl, virt_addr, false, page_size, true);

	if (pml_entry == NULL) {
		log_error("Could not get pagemap entry for address: %#08lx", virt_addr);
		return false;
	}

	pml_entry->entry = 0;
	invlpg((void*)virt_addr);

	return true;
}

bool unmap_page(pte_table_t* toplvl, virt_addr_t virt_addr, size_t flags,
				bool lock) {
	if (lock) {
		try_lock(&vm_lock);
	}

	size_t page_size = get_page_size(flags);

	if (page_size == PAGE_SIZE_1GiB && !pml3_translation) {
		for (size_t i = 0; i < PAGE_SIZE_1GiB; i += PAGE_SIZE_2MiB) {
			if (!unmap_internal(toplvl, virt_addr + i, PAGE_SIZE_2MiB)) {
				return false;
			}
		}

		if (lock) {
			lock_release(&vm_lock);
		}

		return true;
	}

	if (lock) {
		lock_release(&vm_lock);
	}

	return unmap_internal(toplvl, virt_addr, page_size);
}

bool setflags_page(pte_table_t* toplvl, virt_addr_t virt_addr, size_t flags,
				   size_t cache, bool lock) {
	if (lock) {
		try_lock(&vm_lock);
	}

	size_t page_size = get_page_size(flags);
	pte_t* pml_entry = virt_to_pte(toplvl, virt_addr, true, page_size, true);

	if (pml_entry == NULL) {
		log_error("Could not get pagemap entry for address: %#08lx", virt_addr);
		return false;
	}

	size_t real_flags =
		parse_vmm_flags(flags) | parse_cache(cache, page_size > PAGE_SIZE_4KiB);
	phys_addr_t address = pte_get_address(pml_entry);

	pml_entry->entry = 0;
	pte_set_address(pml_entry, address);
	pte_set_flags(pml_entry, real_flags);

	if (lock) {
		lock_release(&vm_lock);
	}

	return true;
}

bool map_pages(pte_table_t* toplvl, phys_addr_t phys_addr,
			   virt_addr_t virt_addr, size_t size, size_t flags, size_t cache) {
	size_t page_size = get_page_size(flags);

	for (size_t i = 0; i < size; i += page_size) {
		if (!map_page(toplvl, phys_addr + i, virt_addr + i, flags, cache,
					  true)) {
			unmap_pages(toplvl, virt_addr, i - page_size, 0);
			return false;
		}
	}

	return true;
}

bool unmap_pages(pte_table_t* toplvl, virt_addr_t virt_addr, size_t size,
				 size_t flags) {
	size_t page_size = get_page_size(flags);

	for (size_t i = 0; i < size; i += page_size) {
		if (!unmap_page(toplvl, virt_addr + i, flags, true)) {
			return false;
		}
	}

	return true;
}

bool remap_page(pte_table_t* toplvl, virt_addr_t new_virt_addr,
				virt_addr_t old_virt_addr, size_t flags, size_t cache) {
	phys_addr_t phys_addr = virt_to_phys_addr(toplvl, old_virt_addr, flags);
	unmap_page(toplvl, old_virt_addr, flags, true);
	return map_page(toplvl, phys_addr, new_virt_addr, flags, cache, true);
}

bool remap_pages(pte_table_t* toplvl, virt_addr_t new_virt_addr,
				 virt_addr_t old_virt_addr, size_t size, size_t flags,
				 size_t cache) {
	size_t page_size = get_page_size(flags);

	for (size_t i = 0; i < size; i += page_size) {
		if (!remap_page(toplvl, new_virt_addr + i, old_virt_addr + i, flags,
						cache)) {
			return false;
		}
	}

	return true;
}

bool setflags_pages(pte_table_t* toplvl, virt_addr_t virt_addr, size_t flags,
					size_t size, size_t cache) {
	size_t page_size = get_page_size(flags);

	for (size_t i = 0; i < size; i += page_size) {
		if (!setflags_page(toplvl, virt_addr, flags, cache, true)) {
			return false;
		}
	}

	return true;
}

void load_paging(pte_table_t* toplvl) {
	write_cr3(from_higher_half((virt_addr_t)toplvl));
}

pte_table_t* save_paging(void) {
	return (pte_table_t*)(to_higher_half(read_cr3()));
}

vm_flags_t parse_vm_flags(size_t flags, bool large_pages) {
	uint64_t patbit = (large_pages ? (1 << 12) : (1 << 7));

	size_t flag = 0;
	size_t cache = 0;

	if (flags & PTE_FLAG_PRESENT) {
		flag |= VMM_FLAG_READ;
	}

	if (flags & PTE_FLAG_WRITABLE) {
		flag |= VMM_FLAG_WRITE;
	}

	if (!(flags & PTE_FLAG_NO_EXECUTE)) {
		flag |= VMM_FLAG_EXECUTE;
	}

	if (flags & PTE_FLAG_USER_ACCESSIBLE) {
		flag |= VMM_FLAG_USER;
	}

	if (flags & PTE_FLAG_GLOBAL) {
		flag |= VMM_FLAG_GLOBAL;
	}

#define MATCH_FLAGS(flags, match) (((flags) & (match)) == (match))
	if (MATCH_FLAGS(flags,
					patbit | PTE_FLAG_NO_CACHE | PTE_FLAG_WRITE_THROUGH)) {
		cache = VMM_CACHE_UNCACHEABLE;
	} else if (MATCH_FLAGS(flags, patbit | PTE_FLAG_NO_CACHE)) {
		cache = VMM_CACHE_WRITE_BACK;
	} else if (MATCH_FLAGS(flags, patbit | PTE_FLAG_WRITE_THROUGH)) {
		cache = VMM_CACHE_WRITE_PROTECTED;
	} else if (MATCH_FLAGS(flags, patbit)) {
		cache = VMM_CACHE_WRITE_THROUGH;
	} else if (MATCH_FLAGS(flags, PTE_FLAG_NO_CACHE | PTE_FLAG_WRITE_THROUGH)) {
		cache = VMM_CACHE_WRITE_COMBINING;
	} else if (MATCH_FLAGS(flags, PTE_FLAG_NO_CACHE)) {
		cache = VMM_CACHE_UNCACHEABLE_STRONG;
	}
#undef MATCH_FLAGS

	return (vm_flags_t){flag, cache};
}

void vm_init(void) {
	cpuid_leaf_t leaf = CPUID_LEAF_INITIALIZER;

	if (cpuid(&leaf, 0x80000001, 0)) {
		pml3_translation = (leaf.edx & (1 << 26)) == (1 << 26);
	}
}

void initialize_pagemap(pte_table_t* toplvl) {
	if (kernel_pagemap == NULL) {
		for (size_t i = (ENTRY_COUNT / 2); i < ENTRY_COUNT; ++i) {
			get_next_pml(toplvl, &toplvl->entries[i], true, -1, -1, -1);
		}

		cpu_enable_pat();
	} else {
		for (size_t i = (ENTRY_COUNT / 2); i < ENTRY_COUNT; ++i) {
			toplvl->entries[i] = kernel_pagemap->entries[i];
		}
	}
}