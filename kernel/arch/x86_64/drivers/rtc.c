#include "utils/time.h"
#include <arch.h>
#include <drivers/rtc.h>
#include <utils/sync.h>

#define RTC_BANK_SIZE 128
#define RTC_PORT_COUNT 2
#define RTC_PORT_BASE 0x70

#define HOUR_PM_BIT (1 << 7)

bool rtc_is_bcd = false;
bool rtc_is_24_hour = false;
struct lock rtc_lock = LOCK_INITIALIZER;

enum rtc_registers {
	Reg_Seconds,
	Reg_Seconds_Alarm,
	Reg_Minutes,
	Reg_Minutes_Alarm,
	Reg_Hours,
	Reg_Hours_Alarm,
	Reg_Day_Of_Week,
	Reg_Day_Of_Month,
	Reg_Month,
	Reg_Year,
	Reg_A,
	Reg_B,
	Reg_C,
	Reg_D,
};

enum register_a {
	Bank_Control = (1 << 4),
	Update_In_Progress = (1 << 7),
};

enum register_b {
	Enable_Daylight_Saving = (1 << 0),
	Hour_Mode_24 = (1 << 1),
	Data_Mode = (1 << 2),
	Enable_Square_Wave = (1 << 3),
	Enable_Update_Ended_Int = (1 << 4),
	Enable_Alarm_Int = (1 << 5),
	Enable_Periodic_Int = (1 << 6),
	Set_Clock = (1 << 7),
};

enum register_c {
	Update_Ended_Interrupt = (1 << 4),
	Alarm_Interrupt = (1 << 5),
	Periodic_Interrupt = (1 << 6),
	Interrupt_Request = (1 << 7),
};

enum register_d { Valid_Cmos = (1 << 7) };

uint8_t to_bcd(uint8_t binary) {
	return (uint8_t)(((binary / 10) << 4) | (binary % 10));
}

uint8_t from_bcd(uint8_t bcd) {
	return ((bcd >> 4) * 10) + (bcd & 0xf);
}

uint16_t rtc_index(uint16_t bank) {
	return bank * 2;
}

uint16_t rtc_data(uint16_t bank) {
	return (bank * 2) + 1;
}

uint8_t rtc_read_reg_raw(enum rtc_registers reg) {
	outpb(RTC_PORT_BASE + rtc_index(0), reg);
	return inpb(RTC_PORT_BASE + rtc_data(0));
}

void rtc_write_reg_raw(enum rtc_registers reg, uint8_t val) {
	outpb(RTC_PORT_BASE + rtc_index(0), reg);
	outpb(RTC_PORT_BASE + rtc_data(0), val);
}

uint8_t rtc_read_reg(enum rtc_registers reg) {
	uint8_t val = rtc_read_reg_raw(reg);
	return rtc_is_bcd ? from_bcd(val) : val;
}

void rtc_write_reg(enum rtc_registers reg, uint8_t val) {
	rtc_write_reg_raw(reg, rtc_is_bcd ? to_bcd(val) : val);
}

uint8_t rtc_read_hour(void) {
	uint8_t data = rtc_read_reg_raw(Reg_Hours);

	bool pm = data & HOUR_PM_BIT;
	data &= ~HOUR_PM_BIT;

	uint8_t hour = rtc_is_bcd ? from_bcd(data) : data;

	if (rtc_is_24_hour) {
		return hour;
	}

	if (pm) {
		hour += 12;
	}

	switch (hour) {
		case 24:
			return 12;
		case 12:
			return 0;
		default:
			return hour;
	}
}

void rtc_write_hour(uint8_t hour) {
	bool pm = (hour > 11);
	uint8_t data = 0;

	if (!rtc_is_24_hour) {
		if (pm) {
			data |= HOUR_PM_BIT;
			hour -= 12;
		}

		if (hour == 0) {
			hour = 12;
		}
	}

	data |= rtc_is_bcd ? to_bcd(hour) : hour;

	rtc_write_reg_raw(Reg_Hours, data);
}

void check_rtc_mode(void) {
	uint8_t reg_b = rtc_read_reg_raw(Reg_B);

	rtc_is_24_hour = (reg_b & Hour_Mode_24) == Hour_Mode_24;
	rtc_is_bcd = !(reg_b & Data_Mode);
}

struct datetime rtc_read_time(void) {
	try_lock(&rtc_lock);
	struct datetime result;

	check_rtc_mode();

	while (rtc_read_reg_raw(Reg_A) & Update_In_Progress) {}

	result.seconds = rtc_read_reg(Reg_Seconds);
	result.minutes = rtc_read_reg(Reg_Minutes);
	result.hours = rtc_read_hour();

	result.day = rtc_read_reg(Reg_Day_Of_Month);
	result.month = rtc_read_reg(Reg_Month);
	result.year = rtc_read_reg(Reg_Year) + 2000;

	lock_release(&rtc_lock);

	return result;
}

void rtc_write_time(struct datetime datetime) {
	try_lock(&rtc_lock);
	check_rtc_mode();

	rtc_write_reg_raw(Reg_B, rtc_read_reg_raw(Reg_B) | Set_Clock);

	rtc_write_reg(Reg_Seconds, datetime.seconds);
	rtc_write_reg(Reg_Minutes, datetime.minutes);
	rtc_write_hour(datetime.hours);

	rtc_write_reg(Reg_Day_Of_Month, datetime.day);
	rtc_write_reg(Reg_Month, datetime.month);

	rtc_write_reg(Reg_Year, (uint8_t)(datetime.year - 2000));

	rtc_write_reg_raw(Reg_B, rtc_read_reg_raw(Reg_B) & ~Set_Clock);

	lock_release(&rtc_lock);
}