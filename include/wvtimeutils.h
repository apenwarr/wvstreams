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

time_t msecdiff(const struct timeval &a, const struct timeval &b);
struct timeval wvtime();

#endif // __WVTIMEUTILS_H
