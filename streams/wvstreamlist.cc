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
    sets_valid = false;
    sure_thing = NULL;
    setcallback(dist_callback, this);
}


bool WvStreamList::isok() const
{
    return true;  // "error" condition on a list is undefined
}


bool WvStreamList::select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
				bool readable, bool writable, bool isexcept)
{
    sure_thing = NULL;
    sets_valid = false;

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
	    sure_thing = &s;
	    return true;
	}
	
	if (s.isok() 
	    && s.select_setup(r, w, x, max_fd, readable, writable, isexcept))
	{
	    sure_thing = &s;
	    return true;
	}
    }
    
    return false;
}


bool WvStreamList::test_set(fd_set &r, fd_set &w, fd_set &x)
{
    WvStream *s;
    
    sel_r = r;
    sel_w = w;
    sel_x = x;
    sets_valid = true;

    if (sure_thing)
	return true;
    
    Iter i(*this);
    for (i.rewind(); i.cur() && i.next(); )
    {
	s = i.data();
	if (s->isok() && s->test_set(r, w, x))
	{
	    sure_thing = s;
	    return true;
	}
    }
    return false;
}


WvStream *WvStreamList::select_one(int msec_timeout, bool readable,
				   bool writable, bool isexception)
{
    if (select(msec_timeout, readable, writable, isexception))
	return sure_thing;
    return NULL;
}


// distribute the callback() request to all children that select 'true'
void WvStreamList::dist_callback(WvStream &, void *userdata)
{
    WvStreamList &l = *(WvStreamList *)userdata;
    WvStream *s;

    if (!l.sets_valid)
    {
	if (l.sure_thing)
	    l.sure_thing->callback();
	return;
    }
	
    WvStreamList::Iter i(l);
    for (i.rewind(); i.next(); )
    {
	s = i.data();
	if (!s->isok()) continue;

	// FIXME: the select(true,true,true) causes weirdness, but I have 
	// no time to test removing it right now.  Why is select() even
	// called here?
	if (s->test_set(l.sel_r, l.sel_w, l.sel_x)
	    && s->select(0, true, true, true))
	{
	    s->callback();
	}
    }
    
    l.sure_thing = NULL;
    l.sets_valid = false;
}
