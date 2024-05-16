#include "stdio_internal.h"

stdio_flags_t stdio_parse_flags(const char** format) {
	stdio_flags_t flags = 0U;

	do {
		switch (**format) {
			case '0':
				flags |= FLAGS_ZEROPAD;
				(*format)++;
				break;
			case '-':
				flags |= FLAGS_LEFT;
				(*format)++;
				break;
			case '+':
				flags |= FLAGS_PLUS;
				(*format)++;
				break;
			case ' ':
				flags |= FLAGS_SPACE;
				(*format)++;
				break;
			case '#':
				flags |= FLAGS_HASH;
				(*format)++;
				break;
			default:
				return flags;
		}
	} while (true);
}
