/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little time functions...
 */
#include "wvtimeutils.h"

time_t msecdiff(const WvTime &a, const WvTime &b)
{
    time_t secdiff = a.tv_sec - b.tv_sec;
    time_t usecdiff = a.tv_usec - b.tv_usec;
    return secdiff * 1000 + usecdiff / 1000;
}


WvTime wvtime()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv;
}


WvTime msecadd(const WvTime &a, time_t msec)
{
    WvTime b;
    b.tv_sec = a.tv_sec + msec / 1000;
    b.tv_usec = a.tv_usec + (msec % 1000) * 1000;
    normalize(b);
    return b;
}


WvTime tvdiff(const WvTime &a, const WvTime &b)
{
    WvTime c;
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

