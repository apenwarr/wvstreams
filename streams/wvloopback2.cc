/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Implementation of a two-way version of WvLoopback.  See wvloopback2.h.
 */
#include "wvloopback2.h"
#include "wvsocketpair.h"

void wvloopback2(IWvStream *&s1, IWvStream *&s2)
{
    int socks[2];
    
    if (wvsocketpair(SOCK_STREAM, socks))
    {
	int errnum = errno;
	s1 = new WvStream;
	s2 = new WvStream;
	s1->seterr(errnum);
	s2->seterr(errnum);
	return;
    }
    
    WvFdStream *f1 = new WvFdStream(socks[0], socks[0]);
    WvFdStream *f2 = new WvFdStream(socks[1], socks[1]);
    
    f1->set_close_on_exec(true);
    f2->set_close_on_exec(true);
    f1->set_nonblock(true);
    f2->set_nonblock(true);
    
    s1 = f1;
    s2 = f2;
}
