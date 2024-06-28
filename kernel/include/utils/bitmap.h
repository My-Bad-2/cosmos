#ifndef UTILS_BITMAP_H
#define UTILS_BITMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BITMAP_INITIALIZER ((struct bitmap){NULL, 0})

#ifdef __cplusplus
extern "C" {
#endif

struct bitmap {
	uint8_t* buffer;
	size_t size;
};

struct bitmap new_bitmap(uint8_t* buffer, size_t size);

bool bitmap_get(struct bitmap* bitmap, size_t index);
void bitmap_set(struct bitmap* bitmap, size_t index);
void bitmap_clear(struct bitmap* bitmap, size_t index);
void bitmap_fill(struct bitmap* bitmap, bool val);

size_t bitmap_entries(const struct bitmap* bitmap);

#ifdef __cplusplus
}
#endif

#endif	// UTILS_BITMAP_H