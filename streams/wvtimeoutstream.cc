/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * See wvtimeoutstream.h.
 */
#include "wvtimeoutstream.h"

WvTimeoutStream::WvTimeoutStream(time_t msec) :
    ok(true)
{
    alarm(msec);
}

void WvTimeoutStream::execute()
{
    WvStream::execute();

    // reset the alarm if it has gone off
    if (alarm_was_ticking) ok = false;
}
