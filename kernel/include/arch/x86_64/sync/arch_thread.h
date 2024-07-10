#ifndef SYNC_ARCH_THREAD_H
#define SYNC_ARCH_THREAD_H

#include <memory/addr.h>

struct arch_thread {
	virt_addr_t sp;
	virt_addr_t* fpu_states;
	uint8_t fpu_buffer[512 * 16];
};

#endif	// SYNC_ARCH_THREAD_H