#ifndef UTILS_MMIO_H
#define UTILS_MMIO_H

#include <stdint.h>

#define mmio_out8(addr, value) (*(volatile uint8_t*)(addr) = (value))
#define mmio_out16(addr, value) (*(volatile uint16_t*)(addr) = (value))
#define mmio_out32(addr, value) (*(volatile uint32_t*)(addr) = (value))
#define mmio_out64(addr, value) (*(volatile uint64_t*)(addr) = (value))

#define mmio_in8(addr) (*(volatile uint8_t*)(addr))
#define mmio_in16(addr) (*(volatile uint16_t*)(addr))
#define mmio_in32(addr) (*(volatile uint32_t*)(addr))
#define mmio_in64(addr) (*(volatile uint64_t*)(addr))

#endif	// UTILS_MMIO_H