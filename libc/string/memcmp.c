#include <stdint.h>
#include <string.h>

int memcmp(const void* lhs, const void* rhs, size_t count) {
	if (count == 0) {
		return 0;
	}

	register const uint8_t* str1 = lhs;
	register const uint8_t* str2 = rhs;

	do {
		if (*str1++ != *str2++) {
			return *--str1 - *--str2;
		}
	} while (--count != 0);

	return 0;
}