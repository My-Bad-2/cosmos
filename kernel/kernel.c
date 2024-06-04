#include <arch.h>
#include <kernel.h>
#include <log.h>
#include <memory/memory.h>

// clang-format off

__attribute__((section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".limine_requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = NULL,
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
	.response = NULL,
};

__attribute__((used, section(".limine_requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// clang-format on

void kmain() {
	arch_init();
	memory_init();

	if (!LIMINE_BASE_REVISION_SUPPORTED) {
		log_info("Limine base revision not supported.");
	}

	log_info("Hello World!");

	arch_halt(true);
}