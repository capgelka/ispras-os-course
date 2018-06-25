#include <inc/stdio.h>
#include <inc/assert.h>
#include <inc/x86.h>

#ifndef JOS_INC_TIME_H
#define JOS_INC_TIME_H

typedef uint32_t time_t;
typedef uint8_t clockid_t;

#define NANOSECONDS 1000000000
#define ENABLE_POSIX_SETTIME_RESTRICTION 1

struct tm
{
    int tm_sec;                   /* Seconds.     [0-60] */
    int tm_min;                   /* Minutes.     [0-59] */
    int tm_hour;                  /* Hours.       [0-23] */
    int tm_mday;                  /* Day.         [1-31] */
    int tm_mon;                   /* Month.       [0-11] */
    int tm_year;                  /* Year - 1900.  */
};


struct timespec
{
    time_t       tv_sec;
    long long    tv_nsec; //it's just long in doc's but it leads tp overflows
};


bool is_leap_year(int year);

int d_to_s(int d);

int timestamp(const struct tm *time);

void mktime(int time, struct tm *tm);

void print_datetime(struct tm *tm);

void snprint_datetime(char *buf, int size, struct tm *tm);

int clock_init(struct timespec* tp);

int clock_getres(clockid_t clock_id, struct timespec *res);
int clock_gettime(clockid_t clock_id, struct timespec *tp);
int clock_settime(clockid_t clock_id, const struct timespec *tp);
int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec
*rqtp, struct timespec *rmtp);

int timespec_from_timestamp(int time, struct timespec* ts);
int timespecec_from_tm(const struct tm* tm, struct timespec* ts);
int timestamp_from_timespec(const struct timespec* ts);
int mkdate_from_timespec(const struct timespec* ts, struct tm* tm);


#define CLOCK_MONOTONIC 1
#define CLOCK_REALTIME 2
#define CLOCK_PROCESS_CPUTIME_ID 3

#define TIMER_ABSTIME 1

#endif
