#ifndef DRIVERS_ARCH_TIMER_H
#define DRIVERS_ARCH_TIMER_H

#include <utils/time.h>

#define I8253_PIT_CONTROL_REG 0x43
#define I8253_PIT_DATA_REG 0x40

enum clock_source { CLOCK_UNSELECTED, CLOCK_PIT, CLOCK_COUNT };

void arch_timer_init(void);

struct datetime fetch_current_time(void);
size_t fetch_current_time_epoch(void);

#endif	// DRIVERS_ARCH_TIMER_H