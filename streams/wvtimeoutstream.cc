/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * See wvtimeoutstream.h.
 */
#include "wvtimeoutstream.h"

bool WvTimeoutStream::pre_select(SelectInfo &si)
{
    time_t now = getmsec();

    /* We expired. */
    if(now >= timeout)
    {
	return true;
    }

    if((si.msec_timeout < 0) || (si.msec_timeout > timeout - now))
    {
	si.msec_timeout = timeout - now;
    }

    return false;
}

