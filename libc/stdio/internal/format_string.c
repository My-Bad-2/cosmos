#include "stdio_internal.h"
#include <ctype.h>
#include <string.h>

size_t stdio_atou(const char** str) {
	size_t i = 0U;

	while (isdigit(**str)) {
		i = i * 10U + (size_t)(*((*str)++) - '0');
	}

	return i;
}

void stdio_format_string_loop(FILE* fp, const char* format, va_list args) {
#define ADVANCE_IN_FORMAT_STRING(ptr)                                          \
	do {                                                                       \
		(ptr)++;                                                               \
		if (!(*(ptr))) {                                                       \
			return;                                                            \
		}                                                                      \
	} while (0)

	while (*format) {
		if (*format != '%') {
			fputc(*format, fp);
			format++;
			continue;
		}

		ADVANCE_IN_FORMAT_STRING(format);

		stdio_flags_t flags = stdio_parse_flags(&format);

		size_t width = 0U;
		if (isdigit(*format)) {
			width = stdio_atou(&format);
		} else if (*format == '*') {
			const int w = va_arg(args, int);

			if (w < 0) {
				flags |= FLAGS_LEFT;
				width = (size_t)-w;
			} else {
				width = (size_t)w;
			}

			ADVANCE_IN_FORMAT_STRING(format);
		}

		size_t precision = 0U;

		if (*format == '.') {
			flags |= FLAGS_PRECISION;
			ADVANCE_IN_FORMAT_STRING(format);

			if (isdigit(*format)) {
				precision = stdio_atou(&format);
			} else if (*format == '*') {
				const int precise = va_arg(args, int);
				precision = (precision > 0) ? (size_t)precise : 0U;
				ADVANCE_IN_FORMAT_STRING(format);
			}
		}

		switch (*format) {
			case 'I': {
				ADVANCE_IN_FORMAT_STRING(format);

				switch (*format) {
					case '0':
						flags |= FLAGS_INT8;
						ADVANCE_IN_FORMAT_STRING(format);
						break;
					case '1':
						ADVANCE_IN_FORMAT_STRING(format);

						if (*format == '6') {
							format++;
							flags |= FLAGS_INT16;
						}

						break;
					case '3':
						ADVANCE_IN_FORMAT_STRING(format);

						if (*format == '2') {
							ADVANCE_IN_FORMAT_STRING(format);
							flags |= FLAGS_INT32;
						}

						break;
					case '6':
						ADVANCE_IN_FORMAT_STRING(format);

						if (*format == '4') {
							ADVANCE_IN_FORMAT_STRING(format);
							flags |= FLAGS_INT64;
						}

						break;
					default:
						break;
				}
				break;
			}

			case 'l':
				flags |= FLAGS_LONG;
				ADVANCE_IN_FORMAT_STRING(format);

				if (*format == 'l') {
					flags |= FLAGS_LONG_LONG;
					ADVANCE_IN_FORMAT_STRING(format);
				}

				break;
			case 'h':
				flags |= FLAGS_SHORT;
				ADVANCE_IN_FORMAT_STRING(format);

				if (*format == 'h') {
					flags |= FLAGS_CHAR;
					ADVANCE_IN_FORMAT_STRING(format);
				}

				break;
			case 't':
				flags |= (sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG
															: FLAGS_LONG_LONG);
				ADVANCE_IN_FORMAT_STRING(format);
				break;
			case 'j':
				flags |= (sizeof(intmax_t) == sizeof(long) ? FLAGS_LONG
														   : FLAGS_LONG_LONG);
				ADVANCE_IN_FORMAT_STRING(format);
				break;
			case 'z':
				flags |= (sizeof(size_t) == sizeof(long) ? FLAGS_LONG
														 : FLAGS_LONG_LONG);
				ADVANCE_IN_FORMAT_STRING(format);
				break;
			default:
				break;
		}

		switch (*format) {
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
			case 'b': {
				if (*format == 'd' || *format == 'i') {
					flags |= FLAGS_SIGNED;
				}

				unsigned char base;

				if (*format == 'x' || *format == 'X') {
					base = BASE_HEX;
				} else if (*format == 'o') {
					base = BASE_OCTAL;
				} else if (*format == 'b') {
					base = BASE_BINARY;
				} else {
					base = BASE_DECIMAL;
					flags &= ~FLAGS_HASH;
				}

				if (*format == 'X') {
					flags |= FLAGS_UPPERCASE;
				}

				format++;

				if (flags & FLAGS_PRECISION) {
					flags &= ~FLAGS_ZEROPAD;
				}

				if (flags & FLAGS_SIGNED) {
					if (flags & FLAGS_LONG_LONG) {
#if STDIO_SUPPORT_LONG_LONG
						const long long value = va_arg(args, long long);
						stdio_print_integer(fp, STDIO_ABS(value), value < 0,
											base, precision, width, flags);
#endif
					} else if (flags & FLAGS_LONG) {
						const long value = va_arg(args, long);
						stdio_print_integer(fp, STDIO_ABS(value), value < 0,
											base, precision, width, flags);
					} else {
						const int value = (flags & FLAGS_CHAR)
											  ? (signed char)va_arg(args, int)
										  : (flags & FLAGS_SHORT)
											  ? (short int)va_arg(args, int)
											  : va_arg(args, int);

						stdio_print_integer(fp, STDIO_ABS(value), value < 0,
											base, precision, width, flags);
					}
				} else {
					flags &= ~(FLAGS_PLUS | FLAGS_SPACE);

					if (flags & FLAGS_LONG_LONG) {
#if STDIO_SUPPORT_LONG_LONG
						stdio_print_integer(fp,
											(stdio_unsigned_value_t)va_arg(
												args, unsigned long long),
											flags, base, precision, width,
											flags);
#endif
					} else if (flags & FLAGS_LONG) {
						stdio_print_integer(
							fp,
							(stdio_unsigned_value_t)va_arg(args, unsigned long),
							false, base, precision, width, flags);
					} else {
						const unsigned int value =
							(flags & FLAGS_CHAR)
								? (unsigned char)va_arg(args, unsigned int)
							: (flags & FLAGS_SHORT)
								? (unsigned short int)va_arg(args, unsigned int)
								: va_arg(args, unsigned int);

						stdio_print_integer(fp, (stdio_unsigned_value_t)value,
											false, base, precision, width,
											flags);
					}
				}

				break;
			}
			case 'c': {
				size_t l = 1U;

				if (!(flags & FLAGS_LEFT)) {
					while (l++ < width) {
						fputc(' ', fp);
					}
				}

				fputc(va_arg(args, int), fp);

				if (flags & FLAGS_LEFT) {
					while (l++ < width) {
						fputc(' ', fp);
					}
				}

				format++;
				break;
			}

			case 's': {
				const char* p = va_arg(args, char*);

				if (p == NULL) {
					stdio_out_reverse(fp, ")LLUN(", 6, width, flags);
				} else {
					size_t l = strnlen(p, precision ? precision
													: STDIO_MAX_BUFFER_SIZE);

					if (flags & FLAGS_PRECISION) {
						l = (l < precision ? l : precision);
					}

					if (!(flags & FLAGS_LEFT)) {
						while (l++ < width) {
							fputc(' ', fp);
						}
					}

					while ((*p != 0) &&
						   (!(flags & FLAGS_PRECISION) || precision)) {
						fputc(*(p++), fp);
						--precision;
					}

					if (flags & FLAGS_LEFT) {
						while (l++ < width) {
							fputc(' ', fp);
						}
					}
				}

				format++;
				break;
			}

			case 'p': {
				width = sizeof(void*) * 2U + 2;
				flags |= FLAGS_ZEROPAD | FLAGS_POINTER;

				uintptr_t value = (uintptr_t)va_arg(args, void*);

				if (value == (uintptr_t)NULL) {
					stdio_out_reverse(fp, ")LLUN(", 5, width, flags);
				} else {
					stdio_print_integer(fp, (stdio_unsigned_value_t)value,
										false, BASE_HEX, precision, width,
										flags);
				}

				format++;
				break;
			}

			case '%':
				fputc('%', fp);
				format++;
				break;
			case 'n': {
				if (flags & FLAGS_CHAR) {
					*(va_arg(args, char*)) = (char)fp->position;
				} else if (flags & FLAGS_SHORT) {
					*(va_arg(args, short*)) = (short)fp->position;
				} else if (flags & FLAGS_LONG) {
					*(va_arg(args, long*)) = (long)fp->position;
				} else if (flags & FLAGS_LONG_LONG) {
#if STDIO_SUPPORT_LONG_LONG
					*(va_arg(args, long long*)) = (long long)fp->position;
#endif
				} else {
					*(va_arg(args, int*)) = (int)fp->position;
				}

				format++;
				break;
			}
			default:
				fputc(*format, fp);
				format++;
				break;
		}
	}

#undef ADVANCE_IN_FORMAT_STRING
}
