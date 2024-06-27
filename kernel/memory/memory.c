#include <memory/heap.h>
#include <memory/memory.h>
#include <memory/phys.h>
#include <memory/vmm.h>

void memory_init() {
	phys_init();
	vmm_init();
	heap_init();
	dump_phys_info();
}