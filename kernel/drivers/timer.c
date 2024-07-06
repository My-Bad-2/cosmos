#include <drivers/arch_timer.h>
#include <drivers/timer.h>

struct timespec realtime_clock;
struct timespec monotonic_clock;

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