#ifndef DEVICES_SERIALS_H
#define DEVICES_SERIALS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SERIAL_COM_PORT_1 0x3F8
#define SERIAL_COM_PORT_2 0x2F8
#define SERIAL_COM_PORT_3 0x3E8
#define SERIAL_COM_PORT_4 0x2E8

#define SERIAL_COM_DEFAULT SERIAL_COM_PORT_1

bool serials_init(uint16_t port);

void serials_putc(uint16_t port, uint8_t ch);
uint8_t serials_getc(uint16_t port);

size_t serials_write_str(uint16_t port, const char* str);

#ifdef __cplusplus
}
#endif

#endif	// DEVICES_SERIALS_H