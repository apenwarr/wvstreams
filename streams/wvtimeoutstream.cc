/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * See wvtimeoutstream.h.
 */
#include "wvtimeoutstream.h"

bool WvTimeoutStream::pre_select(SelectInfo &si)
{
    int now = getmsec();

    /* We expired. */
    if (now - timeout >= 0)
    {
	return true;
    }

    if ((si.msec_timeout < 0) || (si.msec_timeout > timeout - now))
    {
	si.msec_timeout = timeout - now;
    }

    return false;
}

