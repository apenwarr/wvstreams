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
    bool ready = WvStream::pre_select(si);

    if (ms_per_tick)
    {
	now = wvtime();

	if (next < now)
	{
	    si.msec_timeout = 0;
	    return true;
	}

	diff = msecdiff(next, now);
	diff = diff < 0 ? 0 : diff;
	if (diff < si.msec_timeout || si.msec_timeout < 0)
	    si.msec_timeout = diff;
    }

    return ready;
}


bool WvTimeStream::post_select(SelectInfo &si)
{
    return WvStream::post_select(si) || (next < wvtime());
}


void WvTimeStream::execute()
{
    WvStream::execute();

    /* Schedule our next timer event, unless alarm_is_ticking, which
     * would mean that we're here because someone used alarm() rather
     * than because our timer expired. */
    if (!alarm_was_ticking)
	next = msecadd(next, ms_per_tick);
}
