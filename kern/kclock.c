/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <kern/kclock.h>
#include <inc/stdio.h>

void
rtc_init(void)
{
	nmi_disable();
	// LAB 4: your code here
    outb(IO_RTC_CMND, RTC_BREG);
    uint8_t curr_value_b = inb(IO_RTC_DATA);
    outb(IO_RTC_DATA, curr_value_b |= 0b01000000);

    outb(IO_RTC_CMND, RTC_AREG);
    uint8_t curr_value_a = inb(IO_RTC_DATA);
    outb(IO_RTC_DATA, curr_value_a |= 0x0F);

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