/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * See wvtimestream.h.
 */
#include "wvtimestream.h"

WvTimeStream::WvTimeStream():
    next(wvtime()), ms_per_tick(0)
{
}


void WvTimeStream::set_timer(time_t msec)
{
    ms_per_tick = msec > 0 ? msec : 0;
    next = msecadd(wvtime(), ms_per_tick);
}


bool WvTimeStream::isok() const
{
    return true;
}


bool WvTimeStream::pre_select(SelectInfo &si)
{
    WvTime now;
    time_t diff;

    if (!ms_per_tick)
	return false;

    now = wvtime();

    if (next < now)
	return true;

    diff = msecdiff(next, now);
    diff = diff < 0 ? 0 : diff;
    if (diff < si.msec_timeout || si.msec_timeout < 0)
	si.msec_timeout = diff;

    return false;
}


bool WvTimeStream::post_select(SelectInfo &si)
{
    return (next < wvtime());
}


void WvTimeStream::execute()
{
    WvStream::execute();

    // we've got a tick, let's schedule the next one
    next = msecadd(next, ms_per_tick);
}
