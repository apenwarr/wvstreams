/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little time functions...
 */
#include "wvtimeutils.h"

time_t msecdiff(struct timeval &a, struct timeval &b)
{
    time_t secdiff, usecdiff;
    
    secdiff = a.tv_sec - b.tv_sec;
    usecdiff = a.tv_usec - b.tv_usec;
    
    return secdiff*1000 + usecdiff/1000;
}


struct timeval wvtime()
{
    struct timeval tv;
    struct timezone tz;
    
    gettimeofday(&tv, &tz);
    
    return tv;
}
