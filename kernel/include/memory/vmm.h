#ifndef MEMORY_VMM_H
#define MEMORY_VMM_H

#include <memory/addr.h>

#define VMM_FLAG_READ (1 << 0)
#define VMM_FLAG_WRITE (1 << 1)
#define VMM_FLAG_EXECUTE (1 << 2)
#define VMM_FLAG_USER (1 << 3)
#define VMM_FLAG_GLOBAL (1 << 4)
#define VMM_FLAG_LARGE_PAGE (1 << 5)
#define VMM_FLAG_HUGE_PAGE (1 << 6)

#define VMM_FLAG_RW (VMM_FLAG_READ | VMM_FLAG_WRITE)
#define VMM_FLAG_RWX (VMM_FLAG_RW | VMM_FLAG_EXECUTE)

#define VMM_CACHE_UNCACHEABLE (0)
#define VMM_CACHE_UNCACHEABLE_STRONG (1)
#define VMM_CACHE_WRITE_THROUGH (2)
#define VMM_CACHE_WRITE_PROTECTED (3)
#define VMM_CACHE_WRITE_COMBINING (4)
#define VMM_CACHE_WRITE_BACK (5)

#define VMM_CACHE_MMIO VMM_CACHE_UNCACHEABLE_STRONG
#define VMM_CACHE_FRAMEBUFFER VMM_CACHE_WRITE_COMBINING

struct vm_flags {
	size_t flags;
	size_t cache;
};

struct vm_object {
	virt_addr_t base;
	size_t size;
	size_t flags;
};

void vmm_init(void);
void vm_init(void);

struct vm_flags parse_vm_flags(size_t flags, bool large_pages);

#endif	// MEMORY_VMM_H