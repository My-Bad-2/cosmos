#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>
#include <stddef.h>

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void* oldptr, size_t size);
void free(void* ptr);

#endif // _STDLIB_H