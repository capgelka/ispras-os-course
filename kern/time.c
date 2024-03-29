/* See COPYRIGHT for copyright information. */

#include <inc/error.h>

#include <kern/time.h>
#include <kern/tsc.h>

bool is_leap_year(int year)
{
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

int d_to_s(int d)
{
    return d * 24 * 60 * 60;
}

int timestamp(const struct tm *time)
{
    int result = 0, year, month;
    for (year = 1970; year < time->tm_year + 1970; year++)
    {
        result += d_to_s(365 + is_leap_year(year));
    }
    int months[] = {31, 28 + is_leap_year(year), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    for (month = 0; month < time->tm_mon; month++)
    {
        result += d_to_s(months[month]);
    }
    result += d_to_s(time->tm_mday) + time->tm_hour*60*60 + time->tm_min*60 + time->tm_sec;
    return result;
}

void mktime(int time, struct tm *tm)
{
    int year = 70;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;

    while (time > d_to_s(365 + is_leap_year(1900+year))) {
        time -= d_to_s(365 + is_leap_year(1900+year));
        year++;
    }
    tm->tm_year = year;

    int months[] = {31, 28 + is_leap_year(1900+year), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    while (time > d_to_s(months[month])){
        time -= d_to_s(months[month]);
        month++;
    }
    tm->tm_mon = month;

    while (time > d_to_s(1)) {
        time -= d_to_s(1);
        day++;
    }
    tm->tm_mday = day;

    while (time >= 60*60) {
        time -= 60*60;
        hour++;
    }

    tm->tm_hour = hour;

    while (time >= 60) {
        time -= 60;
        minute++;
    }

    tm->tm_min = minute;
    tm->tm_sec = time;
}

void print_datetime(struct tm *tm)
{
    cprintf("%04d-%02d-%02d %02d:%02d:%02d\n",
        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void snprint_datetime(char *buf, int size, struct tm *tm)
{
    assert(size >= 10 + 1 + 8 + 1);
    snprintf(buf, size,
          "%04d-%02d-%02d %02d:%02d:%02d",
          tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
          tm->tm_hour, tm->tm_min, tm->tm_sec);
}

int normalize_time(struct timespec* tp)
{
    int seconds = tp->tv_nsec / NANOSECONDS;
    if (seconds) {
        tp->tv_sec += seconds;
        tp->tv_nsec = (tp->tv_nsec % NANOSECONDS);
    }
    if (tp->tv_nsec < 0) {
        tp->tv_sec--;
        tp->tv_nsec += NANOSECONDS;
    }
    return 0;
}

int clock_init(struct timespec* tp)
{   
    tp->tv_sec = 0;
    tp->tv_nsec = 0;

    return 0;
}

int check_clock_arg(clockid_t cl)
{
    return (
        cl == CLOCK_MONOTONIC ||
        cl == CLOCK_REALTIME ||
        cl == CLOCK_PROCESS_CPUTIME_ID
    );
}

int set_tp_from_timestamp(struct timespec* tp, int tstamp)
{
    tp->tv_sec = (time_t) tstamp;
    tp->tv_nsec = 0;
    return 0;
}

int timestamp_from_timespec(const struct timespec* ts)
{
    /* ts should never be in not normilezed state by convinence,
     so we don't need to do normalixztion first 
    */
    return ts->tv_sec;
}

int clock_getres(clockid_t clock_id, struct timespec* res)
{
    if (res == NULL && !check_clock_arg(clock_id)) {
        return -E_INVAL;
    }
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

struct timespec* resolution_by_clock(clockid_t clock_id)
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


int allign_by_resolution(struct timespec* tp, struct timespec* res)
{
    long long spec_nanoseconds, res_nanoseconds;
    spec_nanoseconds = tp->tv_nsec + (long long) tp->tv_sec * NANOSECONDS;
    res_nanoseconds = res->tv_nsec + (long long) res->tv_sec * NANOSECONDS;
    spec_nanoseconds = (spec_nanoseconds / res_nanoseconds) * res_nanoseconds;
    tp->tv_nsec = spec_nanoseconds;
    tp->tv_sec = 0;
    normalize_time(tp);
    return 0;
}