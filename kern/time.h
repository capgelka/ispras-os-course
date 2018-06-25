/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_TIME_H
#define JOS_KERN_TIME_H


#include <inc/time.h>

extern long long monotonic_time_start;

int normalize_time(struct timespec* tp);

int check_clock_arg(clockid_t cl);

int set_tp_from_timestamp(struct timespec* tp, int tstamp);

#endif // !JOS_KERN_TIME_H
