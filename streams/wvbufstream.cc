/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvBufStream stores data written by write(), and returns it in read().
 * 
 * See wvbufstream.h.
 */ 
#include "wvbufstream.h"


WvBufStream::WvBufStream()
{
    dead = eof = false; 
    death_notify = NULL;
}


WvBufStream::~WvBufStream()
{
    close();
}


void WvBufStream::close()
{
    dead = true; 
    if (death_notify)
	*death_notify = NULL;
    death_notify = NULL;
    WvStream::close();
}


// if uread() is called, someone has already exhausted inbuf... so now it's
// time to close our stream so they know they're at EOF.
size_t WvBufStream::uread(void *buf, size_t size)
{
    if (eof)
	close();
    return 0; 
}


size_t WvBufStream::uwrite(const void *buf, size_t size)
{
    inbuf.put(buf, size);
    return size;
}


bool WvBufStream::isok() const
{
    return !dead;
}


bool WvBufStream::pre_select(SelectInfo &si)
{
    bool ready = WvStream::pre_select(si) || si.wants.writable || eof;

    if (ready)
	si.msec_timeout = 0;

    return ready;
}


bool WvBufStream::post_select(SelectInfo &si)
{
    return WvStream::post_select(si) || si.wants.writable || eof;
}
