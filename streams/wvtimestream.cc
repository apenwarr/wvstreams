/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * See wvtimestream.h.
 */
#include "wvtimestream.h"

WvTimeStream::WvTimeStream()
{
    ms_per_tick = -1;
}


void WvTimeStream::set_timer(time_t msec)
{
    ms_per_tick = msec ? msec : -1;
    alarm(ms_per_tick);
}


bool WvTimeStream::isok() const
{
    return true;
}


void WvTimeStream::tick()
{
    alarm(ms_per_tick);
}


void WvTimeStream::execute()
{
    WvStream::execute();

    // reset the alarm if it has gone off
    if (alarm_was_ticking)
        alarm(ms_per_tick);
}
