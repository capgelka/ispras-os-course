#include <inc/stdio.h>
#include <inc/assert.h>
#include <inc/x86.h>

#ifndef JOS_INC_TIME_H
#define JOS_INC_TIME_H

typedef uint32_t time_t;
typedef uint8_t clockid_t;

#define NANOSECONDS 1000000

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
    time_t  tv_sec;
    long    tv_nsec;
};


bool is_leap_year(int year);

int d_to_s(int d);

int timestamp(struct tm *time);

void mktime(int time, struct tm *tm);

void print_datetime(struct tm *tm);

void snprint_datetime(char *buf, int size, struct tm *tm);


int clock_init(struct timespec* tp);

int clock_getres(clockid_t clock_id, struct timespec *res);
int clock_gettime(clockid_t clock_id, struct timespec *tp);
int clock_settime(clockid_t clock_id, const struct timespec *tp);
int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec
*rqtp, struct timespec *rmtp);

int dummy_clock_getres(clockid_t clock_id, struct timespec res);

// int normilize_time(struct timespec* tp);

#define CLOCK_MONOTONIC 1
#define CLOCK_REALTIME 2
#define CLOCK_PROCESS_CPUTIME_ID 3

// typedef uint8_t clockid_t;

// int check_clock_arg(clockid_t cl);

// int set_tp_from_timestamp(struct timespec* tp, int tstamp);

// int clock_getres(clockid_t clock_id, struct timespec *res);
// int clock_gettime(clockid_t clock_id, struct timespec *tp);
// int clock_settime(clockid_t clock_id, const struct timespec *tp);
// int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec
// *rqtp, struct timespec *rmtp);

#endif
// В рамках решения индивидуального задания от Вас требуется реализовать
// поддержку функций работы с часами [1,2]
// int clock_getres(clockid_t clock_id, struct timespec *res);
// int clock_gettime(clockid_t clock_id, struct timespec *tp);
// int clock_settime(clockid_t clock_id, const struct timespec *tp);
// int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec
// *rqtp, struct timespec *rmtp);

// для часов [3]
// CLOCK_MONOTONIC
// CLOCK_REALTIME
// CLOCK_PROCESS_CPUTIME_ID

// [1]
// http://pubs.opengroup.org/onlinepubs/9699919799/functions/clock_settime.html
// [2]
// http://pubs.opengroup.org/onlinepubs/9699919799/functions/clock_nanosleep.html
// [3] http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/time.h.html


// Работу над индивидуальным заданием необходимо вести в отдельной ветке itask.
// Всё, что необходимо для выполнения тестов, также надо выложить в эту ветку.
