/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */ 
#ifndef __WVTIMEOUTSTREAM_H
#define __WVTIMEOUTSTREAM_H

#include "wvstream.h"

class WvTimeoutStream: public WvStream
{
    unsigned int timeout;
public:
    WvTimeoutStream(unsigned int msec):
	timeout(getmsec() + msec) {}
    virtual bool isok() const {
	return getmsec() < timeout;
    }
    virtual bool pre_select(SelectInfo &si);
    virtual bool post_select(SelectInfo &si) {
	return false;
    }
private:
    static unsigned int getmsec() {
	static struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    }
};

#endif // __WVTIMEOUTSTREAM_H
