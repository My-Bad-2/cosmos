#include <string.h>
#include <utils/bitmap.h>

#define BITS_IN_DATA_TYPE(type) (sizeof(type) * 8)

#define BYTE_INDEX(index) ((index) / BITS_IN_DATA_TYPE(uint8_t))
#define BIT_INDEX(index) ((index) % BITS_IN_DATA_TYPE(uint8_t))

bitmap_t new_bitmap(uint8_t* buffer, size_t size) {
	return (bitmap_t){
		.buffer = buffer,
		.size = size,
	};
}

bool bitmap_get(bitmap_t* bitmap, size_t index) {
	const size_t byte_index = BYTE_INDEX(index);
	const size_t bit_index = BIT_INDEX(index);

	return bitmap->buffer[byte_index] & (1 << bit_index);
}

void bitmap_set(bitmap_t* bitmap, size_t index) {
	const size_t byte_index = BYTE_INDEX(index);
	const size_t bit_index = BIT_INDEX(index);

	bitmap->buffer[byte_index] |= (1 << bit_index);
}

void bitmap_clear(bitmap_t* bitmap, size_t index) {
	const size_t byte_index = BYTE_INDEX(index);
	const size_t bit_index = BIT_INDEX(index);

	bitmap->buffer[byte_index] &= ~(1 << bit_index);
}

void bitmap_fill(bitmap_t* bitmap, bool val) {
	if (val) {
		memset(bitmap->buffer, 0xFF, bitmap->size);
	} else {
		memset(bitmap->buffer, 0, bitmap->size);
	}
}

size_t bitmap_entries(const bitmap_t* bitmap) {
	return bitmap->size * BITS_IN_DATA_TYPE(uint8_t);
}