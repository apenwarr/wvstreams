/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little time functions...
 */
#include "wvtimeutils.h"

time_t msecdiff(const struct timeval &a, const struct timeval &b)
{
    time_t secdiff = a.tv_sec - b.tv_sec;
    time_t usecdiff = a.tv_usec - b.tv_usec;
    return secdiff * 1000 + usecdiff / 1000;
}


struct timeval wvtime()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv;
}


struct timeval msecadd(const struct timeval &a, time_t msec)
{
    struct timeval b;
    b.tv_sec = a.tv_sec + msec / 1000;
    b.tv_usec = a.tv_usec + (msec % 1000) * 1000;
    normalize(b);
    return b;
}


struct timeval tvdiff(const struct timeval &a,
		      const struct timeval &b)
{
    struct timeval c;
    c.tv_sec = a.tv_sec - b.tv_sec;
    c.tv_usec = a.tv_usec;

    if (b.tv_usec > a.tv_usec)
    {
	c.tv_sec--;
	c.tv_usec += 1000000;
    }

    c.tv_usec -= b.tv_usec;

    normalize(c);
    return c;
}

