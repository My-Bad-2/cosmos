#include <stdio.h>

int vprintf(const char* format, va_list args) {
	return vfprintf(stdout, format, args);
}

int printf(const char* format, ...) {
	va_list args;

	va_start(args, format);
	const int ret = vprintf(format, args);
	va_end(args);

	return ret;
}