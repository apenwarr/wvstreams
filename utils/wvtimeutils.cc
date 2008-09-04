/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little time functions...
 */
#include "wvtimeutils.h"
#include <limits.h>

time_t msecdiff(const WvTime &a, const WvTime &b)
{
    long long secdiff = a.tv_sec - b.tv_sec;
    long long usecdiff = a.tv_usec - b.tv_usec;
    long long msecs = secdiff * 1000 + usecdiff / 1000;

    time_t rval;
    if (msecs > INT_MAX)
	rval = INT_MAX;
    else if (msecs < INT_MIN)
	rval = INT_MIN;
    else
	rval = msecs;
    return rval;
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


static WvTime wvstime_cur = wvtime();


const WvTime &wvstime()
{
    return wvstime_cur;
}


static void do_wvstime_sync(bool forward_only)
{
    if (!forward_only)
    {
	wvstime_cur = wvtime();
    }
    else
    {
	WvTime now = wvtime();
	if (wvstime_cur < now)
	    wvstime_cur = now;
    }
}


void wvstime_sync()
{
    do_wvstime_sync(false);
}


void wvstime_sync_forward()
{
    do_wvstime_sync(true);
}


void wvstime_set(const WvTime &_new_time)
{
    wvstime_cur = _new_time;
}

