#include "stdio_internal.h"

void stdio_append_termination_str(FILE* fp) {
	if (fp->write != NULL || fp->max_chars == 0) {
		return;
	}

	if (fp->buffer == NULL) {
		return;
	}

	size_t null_char_pos =
		(fp->position < fp->max_chars) ? fp->position : (fp->max_chars - 1);
	fp->buffer[null_char_pos] = '\0';
}

int stdio_vsnprintf(FILE* fp, const char* format, va_list args) {
	stdio_format_string_loop(fp, format, args);
	stdio_append_termination_str(fp);

	return (int)fp->position;
}