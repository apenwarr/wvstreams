/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * A WvSplitStream uses two different file descriptors: one for input
 * and another for output.  See wvsplitstream.h.
 * 
 * NOTE: this file is a pain to maintain, because many of these functions
 * are almost (but not quite) exactly like the ones in WvStream.  If
 * WvStream changes, you need to change this too.
 */
#include "wvsplitstream.h"
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>

// a console stream made from stdin/stdout.
static WvSplitStream wvconsole(0,1);
WvStream *wvcon = &wvconsole;


WvSplitStream::WvSplitStream(int _rfd, int _wfd)
	: WvStream(_rfd)
{
    in_progress = 0;
    rfd = _rfd;
    wfd = _wfd;
}


WvSplitStream::WvSplitStream()
	: WvStream()
{
    in_progress = 0;
    rfd = wfd = -1;
}


WvSplitStream::~WvSplitStream()
{
    close();
}


void WvSplitStream::close()
{
    assert(in_progress >= 0);
    
    if (in_progress)
	WvStream::close();
    else
    {
	fd = rfd;
	WvStream::close();
	rfd = fd;
	
	fd = wfd;
	WvStream::close();
	wfd = fd;
    }
}


int WvSplitStream::getfd() const
{
    if (in_progress)
	return fd;
    else
	return rfd;
}


void WvSplitStream::noread()
{
    if (rfd == wfd)
	close();
    else
    {
	::close(rfd);
	rfd = wfd;
    }
}


void WvSplitStream::nowrite()
{
    if (rfd == wfd)
	close();
    else
    {
	::close(wfd);
	wfd = rfd;
    }
}


bool WvSplitStream::isok() const
{
    if (in_progress)
	return fd >= 0;
    else
	return rfd >= 0 && wfd >= 0;
}


bool WvSplitStream::select_setup(SelectInfo &si)
{
    time_t alarmleft = alarm_remaining();
    
    if (alarmleft == 0 && !select_ignores_buffer)
	return true; // alarm has rung
    
    if (si.readable || force.readable)
    {
	if (!select_ignores_buffer && inbuf.used())
	    return true; // already ready
	FD_SET(rfd, &si.read);
    }
    if (si.writable || outbuf.used() || force.writable)
	FD_SET(wfd, &si.write);
    if (si.isexception || force.isexception)
    {
	FD_SET(rfd, &si.except);
	FD_SET(wfd, &si.except);
    }
    
    if (si.max_fd < rfd) si.max_fd = rfd;
    if (si.max_fd < wfd) si.max_fd = wfd;
    
    if (alarmleft >= 0
      && (alarmleft < si.msec_timeout || si.msec_timeout < 0))
	si.msec_timeout = alarmleft;
    
    return false;
}


bool WvSplitStream::test_set(SelectInfo &si)
{
    size_t outbuf_used = outbuf.used();
    
    // flush the output buffer if possible
    if (wfd >= 0 && outbuf_used && FD_ISSET(wfd, &si.write))
	flush(0);
    
    return (rfd >= 0 && (FD_ISSET(rfd, &si.read) 
		      || FD_ISSET(rfd, &si.except)))
	|| (wfd >= 0 && (FD_ISSET(wfd, &si.write) 
			 && (!outbuf_used || si.writable)
		      || FD_ISSET(wfd, &si.except)));
}


size_t WvSplitStream::uwrite(const void *buf, size_t size)
{
    size_t retval;
    
    in_progress++;
    fd = wfd;
    retval = WvStream::uwrite(buf, size);
    wfd = fd;
    in_progress--;
    
    return retval;
}


size_t WvSplitStream::uread(void *buf, size_t size)
{
    size_t retval;
    
    in_progress++;
    fd = rfd;
    retval = WvStream::uread(buf, size);
    rfd = fd;
    in_progress--;
    
    return retval;
}


