/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_TIME_H
#define JOS_KERN_TIME_H


#include <inc/time.h>

extern long long monotonic_time_start;
extern time_t monotonic_time_current;

// bool is_leap_year(int year);
// int d_to_s(int d);

// int timestamp(struct tm *time);

// void mktime(int time, struct tm *tm);

// void print_datetime(struct tm *tm);

// void snprint_datetime(char *buf, int size, struct tm *tm);

int normalize_time(struct timespec* tp);

// #define CLOCK_MONOTONIC 1
// #define CLOCK_REALTIME 2
// #define CLOCK_PROCESS_CPUTIME_ID 3



int check_clock_arg(clockid_t cl);

int set_tp_from_timestamp(struct timespec* tp, int tstamp);

#endif // !JOS_KERN_TIME_H
