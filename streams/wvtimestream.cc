/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 *
 * See wvtimestream.h.
 */
#include "wvtimestream.h"

WvTimeStream::WvTimeStream()
{
    struct timezone tz;
    
    ms_per_tick = 0;
    this_is_a_tick = false;
    gettimeofday(&last_tv, &tz);
}


void WvTimeStream::set_timer(int msec)
{
    ms_per_tick = msec;
}


bool WvTimeStream::isok() const
{
    return true;
}


bool WvTimeStream::select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
			     bool readable, bool writable, bool isexception)
{
    struct timeval tv;
    struct timezone tz;
    
    if (gettimeofday(&tv, &tz) || !ms_per_tick)
	return false;

    if (((tv.tv_sec - last_tv.tv_sec) * 1000
	 + (tv.tv_usec - last_tv.tv_usec) / 1000) > ms_per_tick)
    {
	this_is_a_tick = true;
	
	// it seems obvious to do a last_tv = tv; here -- but that results
	// in a lot of inaccuracy, since the _exact_ delay between ticks
	// is not guaranteed.  Instead, we add ms_per_tick milliseconds onto
	// the last tick time, so we always average out to ms_per_tick.
	last_tv.tv_usec += ms_per_tick * 1000;
	last_tv.tv_sec  += last_tv.tv_usec / 1000000;
	last_tv.tv_usec %= 1000000;
    }
    else
	this_is_a_tick = false;
    
    return this_is_a_tick;
}


bool WvTimeStream::test_set(fd_set &r, fd_set &w, fd_set &x)
{
    return this_is_a_tick;
}


