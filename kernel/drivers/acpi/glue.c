#include "uacpi/status.h"
#include <arch.h>
#include <assert.h>
#include <cpu/interrupts.h>
#include <log.h>
#include <memory/paging.h>
#include <memory/vmm.h>
#include <stdlib.h>
#include <uacpi/kernel_api.h>
#include <utils/mmio.h>
#include <utils/sync.h>

uacpi_status uacpi_kernel_raw_io_read(uacpi_io_addr address,
									  uacpi_u8 byte_width,
									  uacpi_u64* out_value) {
	uint16_t port = address;

	switch (byte_width) {
		case 1:
			*out_value = inpb(port);
			break;
		case 2:
			*out_value = inpw(port);
			break;
		case 4:
			*out_value = inpl(port);
			break;
		default:
			return UACPI_STATUS_INVALID_ARGUMENT;
	}

	return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_io_write(uacpi_io_addr address,
									   uacpi_u8 byte_width,
									   uacpi_u64 in_value) {
	uint16_t port = address;

	switch (byte_width) {
		case 1:
			outpb(port, in_value);
			break;
		case 2:
			outpw(port, in_value);
			break;
		case 4:
			outpl(port, in_value);
			break;
		default:
			return UACPI_STATUS_INVALID_ARGUMENT;
	}

	return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_memory_read(uacpi_phys_addr address,
										  uacpi_u8 byte_width,
										  uacpi_u64* out_value) {
	void* addr = uacpi_kernel_map(address, byte_width);

	switch (byte_width) {
		case 1:
			*out_value = mmio_in8(addr);
			break;
		case 2:
			*out_value = mmio_in16(addr);
			break;
		case 4:
			*out_value = mmio_in32(addr);
			break;
		case 8:
			*out_value = mmio_in64(addr);
			break;
		default:
			uacpi_kernel_unmap(addr, byte_width);
			return UACPI_STATUS_INVALID_ARGUMENT;
	}

	uacpi_kernel_unmap(addr, byte_width);
	return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_memory_write(uacpi_phys_addr address,
										   uacpi_u8 byte_width,
										   uacpi_u64 in_value) {
	void* addr = uacpi_kernel_map(address, byte_width);

	switch (byte_width) {
		case 1:
			mmio_out8(addr, in_value);
			break;
		case 2:
			mmio_out16(addr, in_value);
			break;
		case 4:
			mmio_out32(addr, in_value);
			break;
		case 8:
			mmio_out64(addr, in_value);
			break;
		default:
			uacpi_kernel_unmap(addr, byte_width);
			return UACPI_STATUS_INVALID_ARGUMENT;
	}

	uacpi_kernel_unmap(addr, byte_width);
	return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len,
								 uacpi_handle* out_handle) {
	(void)len;
	*out_handle = (uacpi_handle)base;
	return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
	(void)handle;
}

uacpi_status uacpi_kernel_io_read(uacpi_handle handle, uacpi_size offset,
								  uacpi_u8 byte_width, uacpi_u64* value) {
	uacpi_io_addr addr = (uacpi_io_addr)handle;
	return uacpi_kernel_raw_io_read(addr + offset, byte_width, value);
}

uacpi_status uacpi_kernel_io_write(uacpi_handle handle, uacpi_size offset,
								   uacpi_u8 byte_width, uacpi_u64 value) {
	uacpi_io_addr addr = (uacpi_io_addr)handle;
	return uacpi_kernel_raw_io_write(addr + offset, byte_width, value);
}

void* uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
	size_t page_size = get_required_size(len);
	size_t flags = page_size_flags(page_size);

	phys_addr_t phys_addr = align_down(addr, page_size);
	size_t size = align_up((addr - phys_addr) + len, page_size);

	virt_addr_t virt_addr =
		from_higher_half((virt_addr_t)virt_alloc(size, page_size));
	assert(map_pages(kernel_pagemap, phys_addr, virt_addr, size,
					 VMM_FLAG_RW | flags, VMM_CACHE_MMIO));

	return (uint8_t*)virt_addr + (addr - phys_addr);
}

void uacpi_kernel_unmap(void* addr, uacpi_size len) {
	virt_addr_t phys_addr = (virt_addr_t)addr;
	virt_addr_t virt_addr = align_down(phys_addr, PAGE_SIZE);
	size_t size = align_up((phys_addr - virt_addr) + len, PAGE_SIZE);

	assert(unmap_pages(kernel_pagemap, virt_addr, size, 0));
}

void* uacpi_kernel_alloc(uacpi_size size) {
	return malloc(size);
}

void* uacpi_kernel_calloc(uacpi_size count, uacpi_size size) {
	return calloc(count, size);
}

void uacpi_kernel_free(void* mem) {
	free(mem);
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* str) {
	switch (level) {
		case UACPI_LOG_DEBUG:
			log_debug("[UACPI] %s", str);
			break;
		case UACPI_LOG_TRACE:
			log_trace("[UACPI] %s", str);
			break;
		case UACPI_LOG_INFO:
			log_info("[UACPI] %s", str);
			break;
		case UACPI_LOG_WARN:
			log_warn("[UACPI] %s", str);
			break;
		case UACPI_LOG_ERROR:
			log_error("[UACPI] %s", str);
			break;
		default:
			log_info("[UACPI] %s", str);
			break;
	}
}

uacpi_handle uacpi_kernel_create_mutex() {
	return (uacpi_handle)malloc(sizeof(struct lock));
}

void uacpi_kernel_free_mutex(uacpi_handle handle) {
	free(handle);
}

void uacpi_kernel_release_mutex(uacpi_handle handle) {
	lock_release((struct lock*)handle);
}

uacpi_handle uacpi_kernel_create_spinlock() {
	return uacpi_kernel_create_mutex();
}

void uacpi_kernel_free_spinlock(uacpi_handle handle) {
	uacpi_kernel_free_mutex(handle);
}

uacpi_cpu_flags uacpi_kernel_spinlock_lock(uacpi_handle handle) {
	try_lock((struct lock*)handle);
	uacpi_cpu_flags saved_flags = arch_cpu_flags();
	arch_disable_interrupts();
	return (uacpi_cpu_flags)saved_flags;
}

void uacpi_kernel_spinlock_unlock(uacpi_handle handle, uacpi_cpu_flags flags) {
	if (interrupts_enabled(flags)) {
		arch_enable_interrupts();
	}

	lock_release((struct lock*)handle);
}

uacpi_status uacpi_kernel_install_interrupt_handler(
	uacpi_u32 irq, uacpi_interrupt_handler handler, uacpi_handle ctx,
	uacpi_handle* out_irq_handle) {
	allocate_interrupt_handler_at((interrupt_handler_t)handler, irq, ctx);
	clear_interrupt_mask(irq);

	*(size_t*)out_irq_handle = irq;
	return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler,
													  uacpi_handle irq_handle) {
	size_t vector = *(size_t*)irq_handle;
	set_interrupt_mask(vector);

	clear_interrupt_handler(vector);

	return UACPI_STATUS_OK;
}

uacpi_status
uacpi_kernel_handle_firmware_request(uacpi_firmware_request* request) {
	switch (request->type) {
		case UACPI_FIRMWARE_REQUEST_TYPE_BREAKPOINT:
			log_warn("[UACPI] Ignoring AML breakpoint.");
			break;
		case UACPI_FIRMWARE_REQUEST_TYPE_FATAL:
			log_error(
				"[UACPI] Fatal firmware error: type: %u, code: %u, arg: %lu",
				request->fatal.type, request->fatal.code, request->fatal.arg);
			break;
	}

	return UACPI_STATUS_OK;
}