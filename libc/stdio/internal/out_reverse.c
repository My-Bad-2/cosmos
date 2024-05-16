#include "stdio_internal.h"

void stdio_out_reverse(FILE* fp, const char* buffer, size_t len, size_t width,
					   stdio_flags_t flags) {
	const size_t start_pos = fp->position;

	if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
		for (size_t i = len; i < width; i++) {
			fputc(' ', fp);
		}
	}

	while (len) {
		fputc(buffer[--len], fp);
	}

	if (flags & FLAGS_LEFT) {
		while (fp->position - start_pos < width) {
			fputc(' ', fp);
		}
	}
}
