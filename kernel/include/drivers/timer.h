#ifndef DRIVERS_TIMER_H
#define DRIVERS_TIMER_H

#include <utils/time.h>

void timer_init(void);
void timer_sleep_ms(size_t ms);

void timer_handler(size_t ns);

#endif // DRIVERS_TIMER_H