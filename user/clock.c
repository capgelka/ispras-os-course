#include <inc/types.h>
#include <inc/time.h>
#include <inc/stdio.h>
#include <inc/lib.h>

// #define CLOCK_MONOTONIC 1
// #define CLOCK_REALTIME 2
// #define CLOCK_PROCESS_CPUTIME_ID 3

void view_tc(struct timespec* tp) {
	cprintf(
		"show time spec:\n\tseconds: %d\n\tnanoseconds: %lld\n",
		tp->tv_sec,
		tp->tv_nsec
	);
}

void umain(int argc, char **argv) {
    char time[20];

    struct timespec tr;
    struct timespec tc;
    struct timespec tc2;

    clock_init(&tr);
    clock_init(&tc);
    clock_init(&tc2);


    cprintf("CLOCK_MONOTONIC tests:\n");
    cprintf("\n====test clock_getres====\n");
    clock_getres(CLOCK_MONOTONIC, &tr);
    view_tc(&tr);

    cprintf("\n====test clock_gettime/clock_settime====\n");
    clock_gettime(CLOCK_MONOTONIC, &tc);
    cprintf("actual monotonic time");
    view_tc(&tc);
    tc2.tv_sec = 5;
    cprintf("Change seconds to 5. actual monotonic time should have 5 secs\n");
    assert(!clock_settime(CLOCK_MONOTONIC, &tc2));
    clock_gettime(CLOCK_MONOTONIC, &tc);
    assert(tc.tv_sec == 5);
    // sys_yield();
    view_tc(&tc);

    clock_init(&tr);
    clock_init(&tc);
    clock_init(&tc2);


    cprintf("\n\nCLOCK_REALTIME tests:\n");
    cprintf("\n====test clock_getres====\n");
    clock_getres(CLOCK_REALTIME, &tr);
    view_tc(&tr);

    cprintf("\n====test clock_gettime/clock_settime====\n");
    cprintf("actual real time");
    clock_gettime(CLOCK_REALTIME, &tc);
    view_tc(&tc);
    tc2.tv_sec = 10000000;
    cprintf("Change seconds to 5. actual real time should have 5 secs\n");
    assert(!clock_settime(CLOCK_REALTIME, &tc2));
    clock_gettime(CLOCK_REALTIME, &tc);

    view_tc(&tc);


    // cprintf("CLOCK_REALTIME tests:\n");

    int now = sys_gettime();
    struct tm tnow;

    mktime(now, &tnow);

    snprint_datetime(time, 20, &tnow);
    cprintf("DATE: %s (%d)\n", time, now);
}
