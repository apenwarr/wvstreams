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
}


bool WvStreamList::isok() const
{
    return true;  // "error" condition on a list is undefined
}


bool WvStreamList::select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
				bool readable, bool writable, bool isexcept)
{
    bool one_dead = false;
	
    sure_thing.zap();

    Iter i(*this);
    for (i.rewind(); i.cur() && i.next(); )
    {
	WvStream &s = i;
	
	if (!s.isok())
	{
	    one_dead = true;
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
    
    return one_dead || !sure_thing.isempty();
}


bool WvStreamList::test_set(fd_set &r, fd_set &w, fd_set &x)
{
    Iter i(*this);
    for (i.rewind(); i.cur() && i.next(); )
    {
	WvStream &s(i);
	if (s.isok() && s.test_set(r, w, x))
	    sure_thing.append(&s, false);
    }
    return !sure_thing.isempty();
}


// distribute the callback() request to all children that select 'true'
#define STREAMTRACE 0
void WvStreamList::execute()
{
#if STREAMTRACE
    static int level = 0;
    fprintf(stderr, "%*sWvStreamList@%p: ", level++, "", this);
#endif
    
    WvStreamListBase::Iter i(sure_thing);
    for (i.rewind(), i.next(); i.cur(); )
    {
	WvStream &s(i);
	
#if STREAMTRACE
	fprintf(stderr, "[%p], ", &s);
#endif
	
	i.unlink();
	s.callback();
	
	// list might have changed!
	i.rewind();
	i.next();
    }
    
    sure_thing.zap();

#if STREAMTRACE
    level--;
    fprintf(stderr, "\n");
#endif
}
