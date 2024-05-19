#include <arch.h>
#include <devices/serials.h>

void arch_pause() {
	asm volatile("pause");
}

void arch_halt(bool interrupts) {
	if (interrupts) {
		while (true) {
			arch_pause();
		}
	} else {
		while (true) {
			arch_disable_interrupts();
			arch_pause();
		}
	}
}

void arch_disable_interrupts() {
	asm volatile("cli");
}

void arch_enable_interrupts() {
	asm volatile("sti");
}

void outpb(uint16_t port, uint8_t val) {
	asm volatile("outb %1, %0" ::"Nd"(port), "a"(val));
}

void outpw(uint16_t port, uint16_t val) {
	asm volatile("outw %1, %0" ::"Nd"(port), "a"(val));
}

void outpl(uint16_t port, uint32_t val) {
	asm volatile("outl %1, %0" ::"Nd"(port), "a"(val));
}

uint8_t inpb(uint16_t port) {
	uint8_t ret = 0;
	asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

uint16_t inpw(uint16_t port) {
	uint16_t ret = 0;
	asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

uint32_t inpl(uint16_t port) {
	uint32_t ret = 0;
	asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void arch_putc(int ch) {
	serials_putc(SERIAL_COM_DEFAULT, ch);
}

size_t arch_writeln(const char* str) {
	return serials_write_str(SERIAL_COM_DEFAULT, str);
}

void arch_init() {
	if (!serials_init(SERIAL_COM_DEFAULT)) {
		// TODO: Disable serial output
	}
}