#include <log.h>
#include <memory/heap.h>
#include <memory/memory.h>
#include <memory/phys.h>
#include <string.h>

#define MAX_SLAB_NUM (10)
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define GET_METADATA(address)                                                  \
	((struct slab_metadata*)((virt_addr_t)address & ~0xfff))
#define GET_SLAB(address)                                                      \
	(((struct slab_header*)((virt_addr_t)address & ~0xfff))->slab)

struct heap_slab {
	struct lock lock;
	virt_addr_t first_free;
	size_t size;
};

struct slab_header {
	struct heap_slab* slab;
};

struct slab_metadata {
	size_t pages;
	size_t size;
};

struct heap_slab heap_slabs[MAX_SLAB_NUM];

void slab_init(struct heap_slab* slab, size_t size) {
	slab->size = size;
	slab->first_free = to_higher_half(alloc_phys_page());

	size_t available = PAGE_SIZE - align_up(sizeof(struct slab_header), size);
	struct slab_header* ptr = (struct slab_header*)(slab->first_free);
	ptr->slab = slab;
	slab->first_free += align_up(sizeof(struct slab_header), size);

	uintptr_t* arr = (uintptr_t*)(slab->first_free);
	size_t max_size = (available / size) - 1;
	size_t factor = size / 8;

	for (size_t i = 0; i < max_size; ++i) {
		arr[i * factor] = (uint64_t)(&arr[(i + 1) * factor]);
	}

	arr[max_size * factor] = 0;
}

void* slab_alloc(struct heap_slab* slab) {
	try_lock(&slab->lock);

	if (slab->first_free == 0) {
		slab_init(slab, slab->size);
	}

	uintptr_t* old_free = (uintptr_t*)(slab->first_free);
	slab->first_free = old_free[0];
	memset(old_free, 0, slab->size);

	lock_release(&slab->lock);

	return old_free;
}

void slab_free(struct heap_slab* slab, void* ptr) {
	if (ptr == NULL) {
		return;
	}

	try_lock(&slab->lock);

	uintptr_t* new_head = (uintptr_t*)ptr;
	new_head[0] = slab->first_free;
	slab->first_free = (uintptr_t)new_head;

	lock_release(&slab->lock);
}

struct heap_slab* slab_for_each(size_t size) {
	for (size_t i = 0; i < MAX_SLAB_NUM; ++i) {
		if (heap_slabs[i].size >= size) {
			return &heap_slabs[i];
		}
	}

	return NULL;
}

void* heap_malloc(size_t size) {
	struct heap_slab* slab = slab_for_each(size);

	if (slab != NULL) {
		return slab_alloc(slab);
	}

	size_t pages = div_roundup(size, PAGE_SIZE);
	void* ptr = (void*)to_higher_half(alloc_phys_pages(pages + 1));

	struct slab_metadata* metadata = (struct slab_metadata*)ptr;
	metadata->pages = pages;
	metadata->size = size;

	return (void*)((uintptr_t)ptr + PAGE_SIZE);
}

void* heap_page_realloc(void* old_ptr, size_t size) {
	struct slab_metadata* metadata =
		(struct slab_metadata*)((virt_addr_t)old_ptr - PAGE_SIZE);
	size_t old_size = metadata->size;

	if (div_roundup(old_size, PAGE_SIZE) == div_roundup(size, PAGE_SIZE)) {
		metadata->size = size;
		return old_ptr;
	}

	if (size == 0) {
		heap_free(old_ptr);
		return NULL;
	}

	if (size < old_size) {
		old_size = size;
	}

	void* new_ptr = heap_malloc(size);

	if (new_ptr == NULL) {
		return old_ptr;
	}

	memcpy(new_ptr, old_ptr, old_size);
	heap_free(old_ptr);

	return new_ptr;
}

void* heap_realloc(void* oldptr, size_t size) {
	if (oldptr == NULL) {
		return heap_malloc(size);
	}

	if (((virt_addr_t)oldptr & 0xFFF) == 0) {
		return heap_page_realloc(oldptr, size);
	}

	struct heap_slab* slab = GET_SLAB(oldptr);
	size_t old_size = slab->size;

	if (size == 0) {
		heap_free(oldptr);
		return NULL;
	}

	if (size < old_size) {
		old_size = size;
	}

	void* new_ptr = heap_malloc(size);

	if (new_ptr == NULL) {
		return oldptr;
	}

	memcpy(new_ptr, oldptr, old_size);
	heap_free(oldptr);

	return new_ptr;
}

void heap_free(void* ptr) {
	if (ptr == NULL) {
		return;
	}

	if (((uintptr_t)ptr & 0xFFF) == 0) {
		struct slab_metadata* metadata = GET_METADATA(ptr);
		return free_phys_pages((void*)from_higher_half((virt_addr_t)metadata),
							   metadata->pages + 1);
	}

	slab_free(GET_SLAB(ptr), ptr);
}

void heap_init(void) {
	slab_init(&heap_slabs[0], 8);
	slab_init(&heap_slabs[1], 16);
	slab_init(&heap_slabs[2], 32);
	slab_init(&heap_slabs[3], 48);
	slab_init(&heap_slabs[4], 64);
	slab_init(&heap_slabs[5], 128);
	slab_init(&heap_slabs[6], 256);
	slab_init(&heap_slabs[7], 512);
	slab_init(&heap_slabs[8], 1024);

	log_info("Initialized Heap!");
}