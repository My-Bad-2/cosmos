#ifndef _STDIO_INTERNAL_H
#define _STDIO_INTERNAL_H

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#define STDIO_SUPPORT_LONG_LONG 1
#define STDIO_LOG10_TAYLOR_TERMS 4

#if STDIO_LOG10_TAYLOR_TERMS <= 1
#error                                                                         \
	"At least one non-constant Taylor expansion is necessary for the log10() calculation"
#endif

#define STDIO_FLOAT_NOTATION_THRESHOLD                                         \
	STDIO_EXPAND_THEN_CONCATENATE(1e, STDIO_MAX_INTEGRAL_DIGITS_FOR_DECIMAL)

#define STDIO_MAX_BUFFER_SIZE (SIZE_MAX)

// internal flag definitions
#define FLAGS_ZEROPAD (1U << 0U)
#define FLAGS_LEFT (1U << 1U)
#define FLAGS_PLUS (1U << 2U)
#define FLAGS_SPACE (1U << 3U)
#define FLAGS_HASH (1U << 4U)
#define FLAGS_UPPERCASE (1U << 5U)
#define FLAGS_CHAR (1U << 6U)
#define FLAGS_SHORT (1U << 7U)
#define FLAGS_INT (1U << 8U)

#define FLAGS_LONG (1U << 9U)
#define FLAGS_LONG_LONG (1U << 10U)
#define FLAGS_PRECISION (1U << 11U)
#define FLAGS_ADAPT_EXP (1U << 12U)
#define FLAGS_POINTER (1U << 13U)

#define FLAGS_SIGNED (1U << 14U)

#define FLAGS_INT8 FLAGS_CHAR

#if (SHRT_MAX == 32767LL)
#define FLAGS_INT16 FLAGS_SHORT
#elif (INT_MAX == 32767LL)
#define FLAGS_INT16 FLAGS_INT
#elif (LONG_MAX == 32767LL)
#define FLAGS_INT16 FLAGS_LONG
#elif (LLONG_MAX == 32767LL)
#define FLAGS_INT16 FLAGS_LONG_LONG
#else
#error "No basic integer type has a size of 16 bits exactly"
#endif

#if (SHRT_MAX == 2147483647LL)
#define FLAGS_INT32 FLAGS_SHORT
#elif (INT_MAX == 2147483647LL)
#define FLAGS_INT32 FLAGS_INT
#elif (LONG_MAX == 2147483647LL)
#define FLAGS_INT32 FLAGS_LONG
#elif (LLONG_MAX == 2147483647LL)
#define FLAGS_INT32 FLAGS_LONG_LONG
#else
#error "No basic integer type has a size of 32 bits exactly"
#endif

#if (SHRT_MAX == 9223372036854775807LL)
#define FLAGS_INT64 FLAGS_SHORT
#elif (INT_MAX == 9223372036854775807LL)
#define FLAGS_INT64 FLAGS_INT
#elif (LONG_MAX == 9223372036854775807LL)
#define FLAGS_INT64 FLAGS_LONG
#elif (LLONG_MAX == 9223372036854775807LL)
#define FLAGS_INT64 FLAGS_LONG_LONG
#else
#error "No basic integer type has a size of 64 bits exactly"
#endif

#define BASE_BINARY 2
#define BASE_OCTAL 8
#define BASE_DECIMAL 10
#define BASE_HEX 16

typedef unsigned int stdio_flags_t;

#if STDIO_SUPPORT_LONG_LONG
typedef unsigned long long stdio_unsigned_value_t;
typedef long long stdio_signed_value_t;
#else
typedef unsigned long stdio_unsigned_value_t;
typedef long stdio_signed_value_t;
#endif

#define STDIO_ABS(x)                                                           \
	((stdio_unsigned_value_t)((x) > 0 ? (x) : -((stdio_signed_value_t)x)))

void stdio_print_integer(FILE* fp, stdio_unsigned_value_t value, bool negative,
						 unsigned char base, size_t precision, size_t width,
						 stdio_flags_t flags);

void stdio_print_integer_finalize(FILE* fp, char* buffer, size_t len,
								  bool negative, unsigned char base,
								  size_t precision, size_t width,
								  stdio_flags_t flags);

void stdio_out_reverse(FILE* fp, const char* buffer, size_t len, size_t width,
					   stdio_flags_t flags);

stdio_flags_t stdio_parse_flags(const char** format);
void stdio_format_string_loop(FILE* fp, const char* format, va_list args);
int stdio_vsnprintf(FILE* fp, const char* format, va_list args);

#endif	// _STDIO_INTERNAL_H