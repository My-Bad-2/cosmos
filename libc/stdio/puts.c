#include <stdio.h>
#include <string.h>

int fputc(int ch, FILE* fp) {
	size_t write_pos = fp->position++;

	if (write_pos >= fp->max_chars) {
		return -EOF;
	}

	if (fp->write != NULL) {
		fp->write(ch, fp->extra_arg);
	} else {
		fp->buffer[write_pos] = ch;
	}

	return ch;
}

int fputs(const char* restrict str, FILE* fp) {
	const size_t length = strlen(str);

	for (size_t i = 0; i < length; ++i) {
		int ret = fputc(str[i], fp);

		if (ret != str[i]) {
			return ret;
		}
	}

	return length;
}

int putchar(int ch) {
	return fputc(ch, stdout);
}

int puts(const char* restrict str) {
	int ret = fputs(str, stdout);
	fputc('\n', stdout);

	return ret;
}