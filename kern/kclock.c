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
    uint8_t curr_value = inb(IO_RTC_DATA);
    cprintf("===== %x\n", curr_value);
    curr_value |= 0b01000000;
    outb(IO_RTC_DATA, curr_value);
    cprintf("===== %x\n", curr_value);

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