#include <stdint.h>
#include <utils/time.h>

#define NS_MAX 1000000000
#define MS_MAX 1000000

static uint8_t time_days_in_month[] = {
	0,
	31,	 // January
	28,	 // February (not leap year)
	31,	 // March
	30,	 // April
	31,	 // May
	30,	 // June
	31,	 // July
	31,	 // August
	30,	 // September
	31,	 // October
	30,	 // November
	31	 // December
};

time_t timespec_to_ns(struct timespec* timespec) {
	return (timespec->seconds * NS_MAX) + timespec->nseconds;
}

time_t timespec_to_ms(struct timespec* timespec) {
	return timespec_to_ns(timespec) / MS_MAX;
}

void timespec_add(struct timespec* lhs, struct timespec* rhs) {
	lhs->seconds += rhs->seconds;
	lhs->nseconds += rhs->nseconds;

	while (lhs->nseconds >= NS_MAX) {
		lhs->seconds++;
		lhs->nseconds -= NS_MAX;
	}
}

void timespec_add_ns(struct timespec* lhs, time_t ns) {
	struct timespec rhs = {0, ns};
	timespec_add(lhs, &rhs);
}

void timespec_sub(struct timespec* lhs, struct timespec* rhs) {
	lhs->seconds -= rhs->seconds;
	lhs->nseconds -= rhs->nseconds;

	while (lhs->nseconds < 0) {
		lhs->seconds--;
		lhs->nseconds += NS_MAX;
	}

	if (lhs->seconds < 0) {
		lhs->seconds = 0;
		lhs->nseconds = 0;
	}
}

void timespec_sub_ns(struct timespec* lhs, time_t ns) {
	struct timespec rhs = {0, ns};
	timespec_sub(lhs, &rhs);
}

bool is_leap_year(uint16_t year) {
	return (((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0);
}

uint16_t days_in_year(uint16_t year) {
	return is_leap_year(year) ? 366 : 365;
}

uint8_t days_in_month(enum calender_months month, uint16_t year) {
	uint8_t days = time_days_in_month[(uint8_t)month];

	if (month == February && is_leap_year(year)) {
		days++;
	}

	return days;
}