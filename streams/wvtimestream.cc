/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * See wvtimestream.h.
 */
#include "wvtimestream.h"

WvTimeStream::WvTimeStream()
{
    struct timezone tz;
    
    ms_per_tick = max_backlog = 0;
    gettimeofday(&last_tv, &tz);
}


void WvTimeStream::set_timer(int msec, int _max_backlog)
{
    struct timezone tz;

    ms_per_tick = msec;
    max_backlog = _max_backlog;
    
    gettimeofday(&last_tv, &tz);
}


bool WvTimeStream::isok() const
{
    return true;
}


bool WvTimeStream::select_setup(SelectInfo &si)
{
    struct timeval tv;
    struct timezone tz;
    time_t tdiff, tinc;
    
    if (gettimeofday(&tv, &tz) || !ms_per_tick)
	return false;
    
    // compensate for "time warps" (someone sets the clock backwards)
    if (tv.tv_sec < last_tv.tv_sec)
    {
	last_tv.tv_sec = tv.tv_sec;
	last_tv.tv_usec = tv.tv_usec;
    }
    
    tdiff = (tv.tv_sec - last_tv.tv_sec) * 1000
	+ (tv.tv_usec - last_tv.tv_usec) / 1000;
    
    if (tdiff / ms_per_tick > max_backlog)
    {
	tinc = tdiff - max_backlog*ms_per_tick;
	last_tv.tv_sec += tdiff / 1000;
	last_tv.tv_usec += tdiff % 1000;
    }
    
    return (tdiff > ms_per_tick);
}


bool WvTimeStream::test_set(SelectInfo &si)
{
    return false; // if you have to ask, then just forget it.
}


void WvTimeStream::tick()
{
    // it seems obvious to do a last_tv = tv; here -- but that results
    // in a lot of inaccuracy, since the _exact_ delay between ticks
    // is not guaranteed.  Instead, we add ms_per_tick milliseconds onto
    // the last tick time, so we always average out to ms_per_tick.
    last_tv.tv_usec += ms_per_tick * 1000;
    last_tv.tv_sec  += last_tv.tv_usec / 1000000;
    last_tv.tv_usec %= 1000000;
}


void WvTimeStream::execute()
{
    WvStream::execute();
    
    // inform the stream that the clock has officially "ticked"
    tick();
}
