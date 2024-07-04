#ifndef KERNEL_H
#define KERNEL_H

#include <limine.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_paging_mode_request paging_mode_request;
extern volatile struct limine_kernel_address_request kernel_address_request;
extern volatile struct limine_kernel_file_request kernel_file_request;
extern volatile struct limine_rsdp_request rsdp_request;

#ifdef __cplusplus
}
#endif

#endif	// KERNEL_H