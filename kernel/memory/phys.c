#include <assert.h>
#include <kernel.h>
#include <log.h>
#include <string.h>

#include <memory/memory.h>
#include <memory/phys.h>

#include <utils/bitmap.h>
#include <utils/math.h>
#include <utils/sync.h>

static struct bitmap phys_bitmap = BITMAP_INITIALIZER;
static struct lock phys_lock = LOCK_INITIALIZER;
static phys_addr_t highest_usable_addr = 0;
static size_t last_index = 0;
static phys_addr_t phys_highest_address = 0;

size_t total_phys_memory = 0;
size_t usable_phys_memory = 0;
size_t used_phys_memory = 0;

static phys_addr_t internal_alloc_pages(size_t limit, size_t count) {
	size_t i = 0;

	while (last_index < limit) {
		if (!(bitmap_get(&phys_bitmap, last_index++))) {
			if (++i == count) {
				size_t page = last_index - count;

				for (size_t i = page; i < last_index; ++i) {
					bitmap_set(&phys_bitmap, i);
				}

				return (page * PAGE_SIZE);
			}
		} else {
			i = 0;
		}
	}

	return INVALID_ADDR;
}

phys_addr_t alloc_phys_pages(size_t count) {
	if (count == 0) {
		return INVALID_ADDR;
	}

	try_lock(&phys_lock);

	size_t i = last_index;
	phys_addr_t ret =
		internal_alloc_pages(highest_usable_addr / PAGE_SIZE, count);

	if (ret == INVALID_ADDR) {
		last_index = 0;
		ret = internal_alloc_pages(i, count);

		if (ret == INVALID_ADDR) {
			log_fatal("Out of Physical Memory!");
		}
	}

	memset((void*)to_higher_half(ret), 0, count * PAGE_SIZE);

	used_phys_memory += count * PAGE_SIZE;

	lock_release(&phys_lock);

	return ret;
}

phys_addr_t alloc_phys_page() {
	return alloc_phys_pages(1);
}

phys_addr_t highest_phys_addr(void) {
	return phys_highest_address;
}

void free_phys_pages(void* address, size_t count) {
	if (address == NULL) {
		return;
	}

	try_lock(&phys_lock);

	size_t page = ((uintptr_t)address / PAGE_SIZE);

	for (size_t i = page; i < (page + count); i++) {
		bitmap_clear(&phys_bitmap, i);
	}

	used_phys_memory -= count * PAGE_SIZE;

	lock_release(&phys_lock);
}

void free_phys_page(void* address) {
	return free_phys_pages(address, 1);
}

void lock_phys_pages(size_t count) {
	if (count == 0) {
		return;
	}

	try_lock(&phys_lock);

	size_t i = last_index;
	phys_addr_t ret =
		internal_alloc_pages(highest_usable_addr / PAGE_SIZE, count);

	if (ret == INVALID_ADDR) {
		last_index = 0;
		ret = internal_alloc_pages(i, count);

		assert_debug(ret != INVALID_ADDR, "Out of Physical Memory!");
	}

	used_phys_memory += count * PAGE_SIZE;

	lock_release(&phys_lock);
}

void lock_phys_page() {
	return lock_phys_pages(1);
}

void phys_init() {
	struct limine_memmap_entry** memmaps = memmap_request.response->entries;
	size_t memmap_count = memmap_request.response->entry_count;

	for (size_t i = 0; i < memmap_count; ++i) {
		phys_addr_t top = memmaps[i]->base + memmaps[i]->length;
		phys_highest_address = MAX(phys_highest_address, top);

		switch (memmaps[i]->type) {
			case LIMINE_MEMMAP_USABLE:
				usable_phys_memory += memmaps[i]->length;
				highest_usable_addr = MAX(highest_usable_addr, top);
				break;
			case LIMINE_MEMMAP_KERNEL_AND_MODULES:
			case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
				used_phys_memory += memmaps[i]->length;
				break;
			default:
				continue;
		}

		total_phys_memory += memmaps[i]->length;
	}

	size_t bitmap_entries = highest_usable_addr / PAGE_SIZE;
	size_t bitmap_size = align_up(bitmap_entries / 8, PAGE_SIZE);
	bitmap_entries = bitmap_size * 8;

	for (size_t i = 0; i < memmap_count; ++i) {
		if (memmaps[i]->type != LIMINE_MEMMAP_USABLE) {
			continue;
		}

		if (memmaps[i]->length >= bitmap_size) {
			phys_bitmap = new_bitmap((uint8_t*)to_higher_half(memmaps[i]->base),
									 bitmap_size);
			bitmap_fill(&phys_bitmap, true);

			memmaps[i]->length -= bitmap_size;
			memmaps[i]->base += bitmap_size;

			used_phys_memory += bitmap_size;
			break;
		}
	}

	for (size_t i = 0; i < memmap_count; ++i) {
		if (memmaps[i]->type != LIMINE_MEMMAP_USABLE) {
			continue;
		}

		for (phys_addr_t j = 0; j < memmaps[i]->length; j += PAGE_SIZE) {
			bitmap_clear(&phys_bitmap, (memmaps[i]->base + j) / PAGE_SIZE);
		}
	}

	log_info("Initialized Physical Memory Manager!");
}

void dump_phys_info(void) {
	log_trace("Total Memory: %lu bytes", total_phys_memory);
	log_trace("Usable Memory: %lu bytes", usable_phys_memory);
	log_trace("Used Memory: %lu bytes", used_phys_memory);
	log_trace("Free Memory: %lu bytes", usable_phys_memory - used_phys_memory);
}