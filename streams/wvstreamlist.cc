/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvStreamList holds a list of WvStream objects -- and its select() and
 * callback() functions know how to handle multiple simultaneous streams.
 */
#include "wvstreamlist.h"


WvStreamList::WvStreamList()
{
    setcallback(dist_callback, this);
}


bool WvStreamList::isok() const
{
    return true;  // "error" condition on a list is undefined
}


bool WvStreamList::select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
				bool readable, bool writable, bool isexcept)
{
    sure_thing.zap();

    Iter i(*this);
    for (i.rewind(); i.cur() && i.next(); )
    {
	WvStream &s = *i.data();
	
	if (!s.isok())
	{
	    i.unlink();
	    continue;
	}
	
	if (readable && !select_ignores_buffer
	    && inbuf.used() && inbuf.used() > queue_min)
	{
	    sure_thing.append(&s, false);
	}
	
	if (s.isok() 
	    && s.select_setup(r, w, x, max_fd, readable, writable, isexcept))
	{
	    sure_thing.append(&s, false);
	}
    }
    
    return sure_thing.isempty() ? false : true;
}


bool WvStreamList::test_set(fd_set &r, fd_set &w, fd_set &x)
{
    WvStream *s;
    
    sel_r = r;
    sel_w = w;
    sel_x = x;

    Iter i(*this);
    for (i.rewind(); i.cur() && i.next(); )
    {
	s = i.data();
	if (s->isok() && s->test_set(r, w, x))
	    sure_thing.append(s, false);
    }
    return !sure_thing.isempty();
}


// distribute the callback() request to all children that select 'true'
void WvStreamList::dist_callback(WvStream &, void *userdata)
{
    WvStreamList &l = *(WvStreamList *)userdata;
    
    WvStreamListBase::Iter i(l.sure_thing);
    for (i.rewind(), i.next(); i.cur(); )
    {
	WvStream *s = i.data();
	i.unlink();
	s->callback();
	
	// list might have changed!
	i.rewind();
	i.next();
    }
	
    l.sure_thing.zap();
}
