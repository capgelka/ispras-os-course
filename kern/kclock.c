/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/time.h>
#include <inc/string.h>

#include <kern/kclock.h>


int
read_clock(struct tm* date)
{
    date->tm_sec = BCD2BIN(mc146818_read(RTC_SEC));
    date->tm_min = BCD2BIN(mc146818_read(RTC_MIN));
    date->tm_hour = BCD2BIN(mc146818_read(RTC_HOUR));
    date->tm_mday = BCD2BIN(mc146818_read(RTC_DAY));
    date->tm_mon = BCD2BIN(mc146818_read(RTC_MON)) - 1;
    date->tm_year = BCD2BIN(mc146818_read(RTC_YEAR));

    return 0;
}


int gettime(void)
{
    struct tm date, control_date;

    nmi_disable();

    do {
        if (mc146818_read(RTC_AREG) & RTC_UPDATE_IN_PROGRESS) {
            continue;
        }

        read_clock(&date);
        read_clock(&control_date);
        
    } while (memcmp(&date, &control_date, sizeof(date)));

	nmi_enable();
	return timestamp(&date);
}

void
rtc_init(void)
{
	nmi_disable();
	// LAB 4: your code here
    outb(IO_RTC_CMND, RTC_BREG);
    outb(IO_RTC_DATA, inb(IO_RTC_DATA) | 0b01000000);

    outb(IO_RTC_CMND, RTC_AREG);
    outb(IO_RTC_DATA, inb(IO_RTC_DATA) | 0x0F);

	nmi_enable();
}

uint8_t
rtc_check_status(void)
{
	uint8_t status = 0;
	// LAB 4: your code here
    outb(IO_RTC_CMND, RTC_CREG);
    status = inb(IO_RTC_DATA);
	return status;
}

unsigned
mc146818_read(unsigned reg)
{
	outb(IO_RTC_CMND, reg);
	return inb(IO_RTC_DATA);
}

void
mc146818_write(unsigned reg, unsigned datum)
{
	outb(IO_RTC_CMND, reg);
	outb(IO_RTC_DATA, datum);
}

