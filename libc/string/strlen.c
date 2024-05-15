#include <string.h>

size_t strlen(const char* s) {
	if (s == NULL) {
		return 0;
	}

	const char* end = s;

	for (; *end != '\0'; ++end) {
		;
	}

	return end - s;
}

size_t strnlen(const char* string, size_t maxlen) {
	size_t length = 0;

	for (length = 0; length < maxlen; length++, string++) {
		if (!*string) {
			break;
		}
	}

	return length;
}