#include <inc/vsyscall.h>
#include <inc/lib.h>

static inline int32_t
vsyscall(int num)
{
	// LAB 12: Your code here.
	return vsys[num];
}

int
vsys_gettime(void)
{
	return vsyscall(VSYS_gettime);
}

struct timespec*
vsys_clock_getres(clockid_t clock_id)
{
    if (!(clock_id == CLOCK_REALTIME || CLOCK_MONOTONIC || CLOCK_PROCESS_CPUTIME_ID)) {
        return NULL;
    }
    if (clock_id != CLOCK_REALTIME) {
        return (struct timespec*) vsys + VSYS_CLOCK_MONOTONIC;
    }
    else {
        return (struct timespec*) vsys + VSYS_CLOCK_REALTIME;
    }
}