/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */ 
#ifndef __WVTIMEOUTSTREAM_H
#define __WVTIMEOUTSTREAM_H

#include "wvstream.h"

/**
 * WvTimeoutStream is a stream that becomes !isok() after a
 * configurable number of milliseconds. It will wake up a select(). It
 * will return true if select()ed and that the timeout has
 * expired. But using it in a WvStreamList will not have it call the
 * callback/execute because the WvStreamList checks whether isok() is
 * true before doing the select().
 */

class WvTimeoutStream: public WvStream
{
    int timeout;
public:
    WvTimeoutStream(int msec):
	timeout(getmsec() + msec) {}
    virtual bool isok() const {
	/* Beware. Comparing them directly *is* simpler, but is not
	 * correct. */
	return getmsec() - timeout < 0;
    }
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si) {
	return false;
    }
private:
    static int getmsec() {
	static struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    }
};

#endif // __WVTIMEOUTSTREAM_H
