#include <inc/types.h>
#include <inc/time.h>
#include <inc/stdio.h>
#include <inc/lib.h>

// #define CLOCK_MONOTONIC 1
// #define CLOCK_REALTIME 2
// #define CLOCK_PROCESS_CPUTIME_ID 3

void view_tc(struct timespec* tp) {
	cprintf(
		"show time spec:\n\tseconds: %d\n\tnanoseconds: %ld\n",
		tp->tv_sec,
		tp->tv_nsec
	);
}

void umain(int argc, char **argv) {
    char time[20];

    struct timespec tr;
    struct timespec tc;

    cprintf("CLOCK_MONOTONIC tests:\n");
    clock_getres(CLOCK_MONOTONIC, &tr);
    view_tc(&tr);
    clock_gettime(CLOCK_MONOTONIC, &tc);
    view_tc(&tc);
    tc.tv_sec = 0;
    clock_settime(CLOCK_MONOTONIC, &tc);
    clock_gettime(CLOCK_MONOTONIC, &tc);
    view_tc(&tc);


    cprintf("CLOCK_REALTIME tests:\n");

    int now = sys_gettime();
    struct tm tnow;

    mktime(now, &tnow);

    snprint_datetime(time, 20, &tnow);
    cprintf("DATE: %s\n", time);
}
