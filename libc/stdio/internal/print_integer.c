#include "stdio_internal.h"

void stdio_print_integer(FILE* fp, stdio_unsigned_value_t value, bool negative,
						 unsigned char base, size_t precision, size_t width,
						 stdio_flags_t flags) {
	char buffer[BUFSIZE];
	size_t len = 0U;

	if (!value) {
		if (!(flags & FLAGS_PRECISION)) {
			buffer[len++] = '0';
			flags &= ~FLAGS_HASH;
		} else if (base == BASE_HEX) {
			flags &= ~FLAGS_HASH;
		}
	} else {
		do {
			const char digit = (char)(value % base);
			buffer[len++] =
				(char)(digit < 10 ? '0' + digit
								  : (flags & FLAGS_UPPERCASE ? 'A' : 'a') +
										digit - 10);
			value /= base;
		} while (value && (len < BUFSIZE));
	}

	stdio_print_integer_finalize(fp, buffer, len, negative, base, precision,
								 width, flags);
}

void stdio_print_integer_finalize(FILE* fp, char* buffer, size_t len,
								  bool negative, unsigned char base,
								  size_t precision, size_t width,
								  stdio_flags_t flags) {
	size_t unpadded_len = len;

	if (!(flags & FLAGS_LEFT)) {
		if (width && (flags & FLAGS_ZEROPAD) &&
			(negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
			width--;
		}

		while ((flags & FLAGS_ZEROPAD) && (len < width) && (len < BUFSIZE)) {
			buffer[len++] = '0';
		}
	}

	while ((len < precision) && (len < BUFSIZE)) {
		buffer[len++] = '0';
	}

	if (base == BASE_OCTAL && (len > unpadded_len)) {
		flags &= ~FLAGS_HASH;
	}

	if (flags & (FLAGS_HASH | FLAGS_POINTER)) {
		if (!(flags & FLAGS_PRECISION) && len &&
			((len == precision) || (len == width))) {
			if (unpadded_len < len) {
				len--;
			}

			if (len && (base == BASE_HEX || base == BASE_BINARY) &&
				(unpadded_len < len)) {
				len--;
			}
		}

		if ((base == BASE_HEX) && !(flags & FLAGS_UPPERCASE) &&
			(len < BUFSIZE)) {
			buffer[len++] = 'x';
		} else if ((base == BASE_HEX) && (flags & FLAGS_UPPERCASE) &&
				   (len < BUFSIZE)) {
			buffer[len++] = 'X';
		} else if ((base == BASE_BINARY) && (len < BUFSIZE)) {
			buffer[len++] = 'b';
		}

		if (len < BUFSIZE) {
			buffer[len++] = '0';
		}
	}

	if (len < BUFSIZE) {
		if (negative) {
			buffer[len++] = '-';
		} else if (flags & FLAGS_PLUS) {
			buffer[len++] = '+';
		} else if (flags & FLAGS_SPACE) {
			buffer[len++] = ' ';
		}
	}

	stdio_out_reverse(fp, buffer, len, width, flags);
}
