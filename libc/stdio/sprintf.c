#include "internal/stdio_internal.h"

FILE stdio_dummy_file(char* buffer, size_t buffer_size) {
	size_t usable_buffer_size = (buffer_size > STDIO_MAX_BUFFER_SIZE)
									? STDIO_MAX_BUFFER_SIZE
									: (size_t)buffer_size;

	FILE result = {
		.buffer = NULL,
		.position = 0,
		.max_chars = 0,
		.extra_arg = NULL,
		.write = NULL,
	};

	if (buffer != NULL) {
		result.buffer = buffer;
		result.max_chars = usable_buffer_size;
	}

	return result;
}

int vsnprintf(char* buffer, size_t n, const char* format, va_list args) {
	FILE dummy = stdio_dummy_file(buffer, n);
	return stdio_vsnprintf(&dummy, format, args);
}

int snprintf(char* buffer, size_t n, const char* format, ...) {
	va_list args;

	va_start(args, format);
	const int ret = vsnprintf(buffer, n, format, args);
	va_end(args);

	return ret;
}

int vsprintf(char* buffer, const char* format, va_list args) {
	return vsnprintf(buffer, STDIO_MAX_BUFFER_SIZE, format, args);
}

int sprintf(char* buffer, const char* format, ...) {
	va_list args;

	va_start(args, format);
	int ret = vsprintf(buffer, format, args);
	va_end(args);

	return ret;
}