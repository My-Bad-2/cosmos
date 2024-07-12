#include <arch.h>
#include <drivers/arch_timer.h>
#include <drivers/timer.h>

struct timespec realtime_clock = TIMESPEC_INITIALIZER;
struct timespec monotonic_clock = TIMESPEC_INITIALIZER;

void timer_init() {
	realtime_clock = initialize_timespec(fetch_current_time_epoch(), 0);
	monotonic_clock = initialize_timespec(fetch_current_time_epoch(), 0);

	arch_timer_init();
}

void timer_handler(size_t ns) {
	struct timespec interval = initialize_timespec(0, ns);

	timespec_add(&realtime_clock, &interval);
	timespec_add(&monotonic_clock, &interval);
}

size_t time_ns(void) {
	return timespec_to_ns(&monotonic_clock);
}

void timer_sleep_ns(size_t ns) {
	size_t target = time_ns() + ns;

	while (time_ns() < target) {
		arch_pause();
	}
}