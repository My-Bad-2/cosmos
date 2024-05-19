#include <stdint.h>
#include <stdio.h>

#define STDIN (0)
#define STDOUT (1)
#define STDERR (2)

extern size_t arch_putc(int ch);

static void stdio_putc(int ch, void* ignored) {
	(void)ignored;

	arch_putc(ch);
}

// disable stdin for now
FILE __streams[] = {
	[STDIN] =
		{
			NULL,
			0,
			0,
			NULL,
			NULL,
		},
	[STDOUT] =
		{
			NULL,
			0,
			SIZE_MAX,
			NULL,
			stdio_putc,
		},
	[STDERR] =
		{
			NULL,
			0,
			SIZE_MAX,
			NULL,
			stdio_putc,
		},
};