#ifndef DRIVERS_RTC_H
#define DRIVERS_RTC_H

#include <utils/time.h>

struct datetime rtc_read_time(void);
void rtc_write_time(struct datetime datetime);

#endif // DRIVERS_RTC_H