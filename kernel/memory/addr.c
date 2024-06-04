#include <kernel.h>
#include <memory/addr.h>

virt_addr_t to_higher_half(phys_addr_t address) {
	return is_higher_half(address) ? address
								   : (address + hhdm_request.response->offset);
}

phys_addr_t from_higher_half(virt_addr_t address) {
	return !is_higher_half(address) ? address
									: (address - hhdm_request.response->offset);
}

bool is_higher_half(uintptr_t address) {
	return address >= hhdm_request.response->offset;
}

uintptr_t align_down(uintptr_t address, size_t page_size) {
	return address & ~(page_size - 1);
}

uintptr_t align_up(uintptr_t address, size_t page_size) {
	return align_down(address + page_size - 1, page_size);
}

bool is_aligned(uintptr_t address, size_t page_size) {
	return (align_down(address, page_size) == address);
}