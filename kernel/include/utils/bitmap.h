#ifndef UTILS_BITMAP_H
#define UTILS_BITMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BITMAP_INITIALIZER ((bitmap_t){NULL, 0})

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t* buffer;
	size_t size;
} bitmap_t;

bitmap_t new_bitmap(uint8_t* buffer, size_t size);

bool bitmap_get(bitmap_t* bitmap, size_t index);
void bitmap_set(bitmap_t* bitmap, size_t index);
void bitmap_clear(bitmap_t* bitmap, size_t index);
void bitmap_fill(bitmap_t* bitmap, bool val);

size_t bitmap_entries(const bitmap_t* bitmap);

#ifdef __cplusplus
}
#endif

#endif	// UTILS_BITMAP_H