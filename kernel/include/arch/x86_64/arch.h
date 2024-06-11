#ifndef ARCH_H
#define ARCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define outp(port, val)                                                        \
	_Generic((val), uint8_t: outpb, uint16_t: outpw, uint32_t: outpl)(port,    \
																	  (val))

void arch_early_init(void);
void arch_init(void);

void arch_pause(void);
void arch_halt(bool interrupts);

void arch_disable_interrupts(void);
void arch_enable_interrupts(void);

void outpb(uint16_t port, uint8_t val);
void outpw(uint16_t port, uint16_t val);
void outpl(uint16_t port, uint32_t val);

uint8_t inpb(uint16_t port);
uint16_t inpw(uint16_t port);
uint32_t inpl(uint16_t port);

void arch_putc(int ch);
size_t arch_writeln(const char* str);

#ifdef __cplusplus
}
#endif

#endif	// ARCH_H