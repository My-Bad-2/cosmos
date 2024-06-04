#include <memory/memory.h>
#include <memory/phys.h>

void memory_init() {
	phys_init();
	dump_phys_info();
}