#include <stdlib.h>

extern void* heap_malloc(size_t size);
extern void* heap_realloc(void* oldptr, size_t size);
extern void heap_free(void* ptr);

void* malloc(size_t size) {
    return heap_malloc(size);
}

void* realloc(void* oldptr, size_t size) {
    return heap_realloc(oldptr, size);
}

void free(void *ptr) {
    heap_free(ptr);
}