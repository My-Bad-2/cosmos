#include <stdlib.h>
#include <string.h>

void* calloc(size_t num, size_t size) {
	void* ptr = malloc(num * size);

	if (ptr == NULL) {
		return NULL;
	}

	memset(ptr, 0, num * size);

	return ptr;
}