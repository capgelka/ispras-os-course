#include <inc/types.h>
#include <inc/time.h>
#include <inc/stdio.h>
#include <inc/lib.h>

// #define CLOCK_MONOTONIC 1
// #define CLOCK_REALTIME 2
// #define CLOCK_PROCESS_CPUTIME_ID 3

void view_tc(struct timespec* tp)
{
	cprintf(
		"show time spec:\n\tseconds: %d\n\tnanoseconds: %lld\n",
		tp->tv_sec,
		tp->tv_nsec
	);
}

void view_date(struct timespec* tp)
{
    struct tm date;
    mkdate_from_timespec(tp, &date);
    print_datetime(&date);
}

void umain(int argc, char **argv)
{
    // char time[20];

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

    tc2.tv_sec = 2;
    cprintf("Change seconds to 2. actual monotonic time should have 2 secs\n");
    assert(!clock_settime(CLOCK_MONOTONIC, &tc2));
    clock_gettime(CLOCK_MONOTONIC, &tc);
    assert(tc.tv_sec == 2);
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
    struct tm date = {
        .tm_sec = 8,
        .tm_min = 57,
        .tm_hour = 20,
        .tm_mday = 25,
        .tm_mon = 8,
        .tm_year = 21
    };
    tc2.tv_sec = timestamp(&date);

    cprintf("Change timestamp to linux birthday. actual real time should be the same\n");
    assert(!clock_settime(CLOCK_REALTIME, &tc2));
    clock_gettime(CLOCK_REALTIME, &tc);
    view_date(&tc);

    view_tc(&tc);
    assert(tc.tv_sec == tc2.tv_sec);


    cprintf("CLOCK_PROCESS_CPUTIME_ID tests:\n");
    cprintf("\n====test clock_getres====\n");
    clock_getres(CLOCK_PROCESS_CPUTIME_ID, &tr);
    view_tc(&tr);

    // int r;

    // for (int i = 0; i< 100; i++) {
    //     if ((r = spawnl("hello", "hello", 0)) < 0)
    //         panic("spawnl(init) failed: %i", r);
    // }
    for (int i = 0; i< 100000000; i++) {
        tc2.tv_nsec = i*i*i;
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tc);
    view_tc(&tc);

    clock_init(&tc2);
    tc2.tv_sec = 3;
    tc2.tv_nsec = 0;
    cprintf("Change seconds to 3. actual process time should have 3 secs\n");
    assert(!clock_settime(CLOCK_PROCESS_CPUTIME_ID, &tc2));
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tc);
    view_tc(&tc);
    assert(tc.tv_sec == tc2.tv_sec);


    tc2.tv_sec = 1;
    cprintf("Change seconds to 1. actual process time should have 1 secs\n");
    assert(!clock_settime(CLOCK_PROCESS_CPUTIME_ID, &tc2));
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tc);
    view_tc(&tc);
    assert(tc.tv_sec == tc2.tv_sec);



    // int now = sys_gettime();
    // struct tm tnow;

    // mktime(now, &tnow);

    // snprint_datetime(time, 20, &tnow);
    // cprintf("DATE: %s (%d)\n", time, now);
}
