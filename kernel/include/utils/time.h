#ifndef UTILS_TIME_H
#define UTILS_TIME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TIMESPEC_INITIALIZER ((struct timespec){0, 0})
#define DATETIME_INITIALIZER ((struct datetime){0, 0, 0, 0, 0, 0})

typedef long time_t;
typedef int64_t ticks_t;

struct datetime {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day;
	uint8_t month;
	uint16_t year;
};

struct timespec {
	time_t seconds;
	time_t nseconds;
};

enum calender_months {
	January = 1,
	February,
	March,
	April,
	May,
	June,
	July,
	August,
	September,
	October,
	November,
	December
};

struct timespec initialize_timespec(time_t sec, long ns);

time_t timespec_to_ns(struct timespec* timespec);
time_t timespec_to_ms(struct timespec* timespec);

void timespec_add(struct timespec* lhs, struct timespec* rhs);
void timespec_add_ns(struct timespec* lhs, time_t ns);

void timespec_sub(struct timespec* lhs, struct timespec* rhs);
void timespec_sub_ns(struct timespec* lhs, time_t ns);

bool is_leap_year(uint16_t year);
uint16_t days_in_year(uint16_t year);
uint8_t days_in_month(enum calender_months month, uint16_t year);

size_t calculate_epoch(struct datetime datetime);

#endif	// UTILS_TIME_H