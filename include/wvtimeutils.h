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

time_t msecdiff(struct timeval &a, struct timeval &b);
struct timeval wvtime();

inline void normalize(struct timeval &tv)
{
    tv.tv_sec += tv.tv_usec / 1000000;
    tv.tv_usec %= 1000000;
}

#endif // __WVTIMEUTILS_H
