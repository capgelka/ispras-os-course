/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/time.h>


#include <kern/kclock.h>
#include <kern/tsc.h>

int
clock_getres(clockid_t clock_id, struct timespec *res) {
    /* our clock get's only seconds. Nothing to do here for REALTIME 
      But we can use tcs for monotonic
    */
    if (clock_id != CLOCK_REALTIME) {
        res->tv_nsec = nanosec_interval();
        res->tv_sec = 0;
    }
    else {
        res->tv_nsec = 0;
        res->tv_sec = 1;
    }
    return 0;
}


int
clock_gettime(clockid_t clock_id, struct timespec *tp) {
    return 0;
}