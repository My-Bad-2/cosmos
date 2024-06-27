#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H

#include <limits.h>
#include <memory/addr.h>
#include <stddef.h>
#include <stdint.h>
#include <utils/sync.h>

#ifdef __cplusplus
extern "C" {
#endif

void* heap_malloc(size_t size);
void* heap_realloc(void* oldptr, size_t size);
void heap_free(void* ptr);

void heap_init(void);

#ifdef __cplusplus
}
#endif

#endif	// MEMORY_HEAP_H