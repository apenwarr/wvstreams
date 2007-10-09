/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Implementation of the WvLoopback stream.  WvLoopback uses a
 * socketpair() to create a stream that allows you to read()
 * everything written to it, even (especially) across a fork() call.
 */
#include "wvloopback.h"
#include "wvsocketpair.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"

WV_LINK(WvLoopback);

static IWvStream *create_loopback(WvStringParm, IObject *)
{
    return new WvLoopback();
}

static WvMoniker<IWvStream> reg("loop",  create_loopback);


WvLoopback::WvLoopback()
{
    int socks[2];
    
    if (wvsocketpair(SOCK_STREAM, socks))
    {
	seterr(errno);
	return;
    }
    
    rfd = socks[0];
    wfd = socks[1];

    set_close_on_exec(true);
    set_nonblock(true);
}
