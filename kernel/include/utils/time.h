#ifndef UTILS_TIME_H
#define UTILS_TIME_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define TIMESPEC_INITIALIZER ((timespec_t){0, 0})
#define DATETIME_INITIALIZER ((datetime_t){0, 0, 0, 0, 0, 0})

typedef long time_t;

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day;
	uint8_t month;
	uint16_t year;
} datetime_t;

typedef struct timespec {
	time_t seconds;
	time_t nseconds;
} timespec_t;

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

time_t timespec_to_ns(timespec_t* timespec);
time_t timespec_to_ms(timespec_t* timespec);

void timespec_add(timespec_t* lhs, timespec_t* rhs);
void timespec_add_ns(timespec_t* lhs, time_t ns);

void timespec_sub(timespec_t* lhs, timespec_t* rhs);
void timespec_sub_ns(timespec_t* lhs, time_t ns);

bool is_leap_year(uint16_t year);
uint16_t days_in_year(uint16_t year);
uint8_t days_in_month(enum calender_months month, uint16_t year);

#endif	// UTILS_TIME_H