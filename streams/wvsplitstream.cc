/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * A WvSplitStream uses two different file descriptors: one for input
 * and another for output.  See wvsplitstream.h.
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


bool WvSplitStream::test_set(fd_set &r, fd_set &w, fd_set &x)
{
    return (FD_ISSET(rfd, &r) || FD_ISSET(wfd, &w)
	    || FD_ISSET(rfd, &x) || FD_ISSET(wfd, &x));
}


bool WvSplitStream::isok() const
{
    if (in_progress)
	return fd >= 0;
    else
	return rfd >= 0 && wfd >= 0;
}


bool WvSplitStream::select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
				 bool readable, bool writable, bool isexcept)
{
    if (readable)
    {
	if (!select_ignores_buffer && inbuf.used())
	    return true; // already ready
	FD_SET(rfd, &r);
    }
    if (writable)
	FD_SET(wfd, &w);
    if (isexcept)
    {
	FD_SET(rfd, &x);
	FD_SET(wfd, &x);
    }
    
    if (max_fd < rfd) max_fd = rfd;
    if (max_fd < wfd) max_fd = wfd;
    
    return false;
}


bool WvSplitStream::select(time_t msec_timeout, bool readable,
			   bool writable, bool isexception)
{
    bool retval;
    in_progress++;
    fd = rfd;
    retval = WvStream::select(msec_timeout, readable, writable, isexception);
    in_progress--;
    return retval;
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


