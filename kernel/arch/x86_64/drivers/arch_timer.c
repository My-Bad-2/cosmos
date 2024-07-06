#include <arch.h>
#include <cpu/interrupts.h>
#include <drivers/arch_timer.h>
#include <drivers/rtc.h>
#include <drivers/timer.h>
#include <log.h>
#include <utils/fixed_point.h>
#include <stdatomic.h>

#define DESIRED_PIT_FREQUENCY 1000

static enum clock_source source = CLOCK_UNSELECTED;
static struct fixed_point us_per_pit;
static atomic_size_t pit_ticks = 0;
static uint16_t pit_divisor = 0;

#define INTERNAL_FREQ 1193182U
#define INTERNAL_FREQ_3X 3579546U

#define INTERNAL_FREQ_TICKS_PER_MS (INTERNAL_FREQ / 1000)

static inline size_t current_ticks_pit(void) {
	return pit_ticks;
}

static void set_pit_frequency(uint32_t frequency) {
	uint32_t count = 0;
	uint32_t remainder = 0;

	if (frequency <= 18) {
		count = 0xffff;
	} else if (frequency >= INTERNAL_FREQ) {
		count = 1;
	} else {
		count = INTERNAL_FREQ_3X / frequency;
		remainder = INTERNAL_FREQ_3X % frequency;

		if (remainder >= (INTERNAL_FREQ_3X / 2)) {
			count++;
		}

		count /= 3;
		remainder = count % 3;

		if (remainder >= 1) {
			count++;
		}
	}

	pit_divisor = count & 0xffff;

	fixed_point_div_32(&us_per_pit, 1000 * 1000 * 3 * count, INTERNAL_FREQ_3X);

	outpb(I8253_PIT_CONTROL_REG, 0x34);
	outpb(I8253_PIT_DATA_REG, (uint8_t)pit_divisor);
	outpb(I8253_PIT_DATA_REG, (uint8_t)(pit_divisor >> 8));
}

void handle_pit_interrupt(struct iframe* frame) {
	(void)frame;

	pit_ticks++;
}

static inline void pit_sleep_ms(size_t ms) {
	volatile size_t target = current_ticks_pit() + ms;

	while (current_ticks_pit() < target) {
		arch_pause();
	}
}

void arch_timer_init(void) {
	source = CLOCK_PIT;

	switch (source) {
		case CLOCK_PIT:
			set_pit_frequency(DESIRED_PIT_FREQUENCY);
			struct interrupt_handler* handler = allocate_handler(IRQ_TIMER, handle_pit_interrupt);
			handler->eoi_first = true;
			clear_interrupt_mask(IRQ_TIMER);
			break;
		default:
			log_fatal("Unknown clock source!");
			break;
	}
}

void timer_sleep_ms(size_t ms) {
	switch (source) {
		case CLOCK_PIT:
			pit_sleep_ms(ms);
			break;
		default:
			log_fatal("Unknown clock source!");
			break;
	}
}

struct datetime fetch_current_time(void) {
	return rtc_read_time();
}

size_t fetch_current_time_epoch(void) {
	return calculate_epoch(fetch_current_time());
}