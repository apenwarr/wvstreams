/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little time functions...
 *
 */
#ifndef __WVTIMEUTILS_H
#define __WVTIMEUTILS_H


#ifdef _WIN32
#include "winsock2.h"
#include <time.h>
#else
#include <sys/time.h>
#endif

typedef struct timeval WvTime;

static const WvTime wvtime_zero = {
    0, 0
};

/** Returns the number of milliseconds between times a and b. */
time_t msecdiff(const WvTime &a, const WvTime &b);

/** Returns the current time of day. */
WvTime wvtime();

/** Adds the specified number of milliseconds to a time value. */
WvTime msecadd(const WvTime &a, time_t msec);

/** Returns the timeval difference between two timevals. */
WvTime tvdiff(const WvTime &a, const WvTime &b);

/** Normalizes the time value. */
inline void normalize(WvTime &tv)
{
    tv.tv_sec += tv.tv_usec / 1000000;
    tv.tv_usec %= 1000000;
}

/** Compares two time values. */
inline bool operator< (const WvTime &a,
    const struct timeval &b)
{
    return a.tv_sec < b.tv_sec || (a.tv_sec == b.tv_sec
        && a.tv_usec < b.tv_usec);
}

inline bool operator== (const WvTime &a,
    const struct timeval &b)
{
    return a.tv_sec == b.tv_sec && a.tv_usec == b.tv_usec;
}

#endif // __WVTIMEUTILS_H
