/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little time functions...
 *
 */
#ifndef __WVTIMEUTILS_H
#define __WVTIMEUTILS_H

#include <sys/time.h>

/**
 * Returns the number of milliseconds between times a and b.
 */
time_t msecdiff(const struct timeval &a, const struct timeval &b);

/**
 * Returns the current time of day.
 */
struct timeval wvtime();

/**
 * Adds the specified number of milliseconds to a time value.
 */
struct timeval msecadd(const struct timeval &a, time_t msec);

/**
 * Normalizes the time value.
 */
inline void normalize(struct timeval &tv)
{
    tv.tv_sec += tv.tv_usec / 1000000;
    tv.tv_usec %= 1000000;
}

#endif // __WVTIMEUTILS_H
