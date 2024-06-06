#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#define PAGE_SIZE (0x1000)

#define PAGE_SIZE_4KiB (0x1000ULL)
#define PAGE_SIZE_2MiB (0x200000ULL)
#define PAGE_SIZE_1GiB (0x40000000ULL)

void memory_init();

#ifdef __cplusplus
}
#endif

#endif	// MEMORY_MEMORY_H