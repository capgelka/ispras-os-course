#include <inc/time.h>
// #include <lib/syscall.h>
#include <inc/lib.h>

bool is_leap_year(int year)
{
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

int d_to_s(int d)
{
    return d * 24 * 60 * 60;
}

int timestamp(struct tm *time)
{
    int result = 0, year, month;
    for (year = 1970; year < time->tm_year + 2000; year++)
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

int clock_getres(clockid_t clock_id, struct timespec *res)
{
    return sys_clock_getres(clock_id, res);
}


int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    return sys_clock_gettime(clock_id, tp);
}

int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
    return sys_clock_settime(clock_id, tp);
}

int clock_nanosleep(
    clockid_t clock_id,
    int flags,
    const struct timespec *rqtp, 
    struct timespec *rmtp
)
{
    return sys_clock_nanosleep(clock_id, flags, rqtp, rmtp);
}
