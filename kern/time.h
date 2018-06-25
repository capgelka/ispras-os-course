/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_TIME_H
#define JOS_KERN_TIME_H


#include <inc/time.h>
#include <inc/vsyscall.h>
#include <kern/vsyscall.h>

extern long long monotonic_time_start;

int normalize_time(struct timespec* tp);

int check_clock_arg(clockid_t cl);

int set_tp_from_timestamp(struct timespec* tp, int tstamp);

int clock_getres(clockid_t clock_id, struct timespec* res);

struct timespec* resolution_by_clock(clockid_t clock_id);

int allign_by_resolution(struct timespec* tp, struct timespec* res);

#endif // !JOS_KERN_TIME_H
