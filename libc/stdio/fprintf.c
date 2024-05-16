#include "internal/stdio_internal.h"

int vfprintf(FILE* fp, const char* format, va_list args) {
	return stdio_vsnprintf(fp, format, args);
}

int fprintf(FILE* fp, const char* format, ...) {
	va_list args;

	va_start(args, format);
	int ret = vfprintf(fp, format, args);
	va_end(args);

	return ret;
}
