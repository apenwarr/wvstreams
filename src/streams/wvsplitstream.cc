/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
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
    rfd = _rfd;
    wfd = _wfd;
}


WvSplitStream::WvSplitStream()
	: WvStream()
{
    rfd = wfd = -1;
}


WvSplitStream::~WvSplitStream()
{
    close();
}


void WvSplitStream::close()
{
    WvStream::close();
    rfd = -1;
    wfd = -1;
}


int WvSplitStream::getrfd() const
{
    return rfd;
}


int WvSplitStream::getwfd() const
{
    return wfd;
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

