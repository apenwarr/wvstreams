/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvStreamList holds a list of WvStream objects -- and its select() and
 * callback() functions know how to handle multiple simultaneous streams.
 */
#include "wvstreamlist.h"

// enable this to add some read/write trace messages (this can be VERY
// verbose)
#define STREAMTRACE 0
#if STREAMTRACE
# define TRACE(x, y...) fprintf(stderr, x, ## y)
#else
# define TRACE(x, y...)
#endif

WvStreamList::WvStreamList()
{
    auto_prune = true;
}


WvStreamList::~WvStreamList()
{
    // nothing to do
}


bool WvStreamList::isok() const
{
    return true;  // "error" condition on a list is undefined
}


bool WvStreamList::select_setup(SelectInfo &si)
{
    bool one_dead = false;
    bool oldrd, oldwr, oldex, oldforce;
    
    // usually because of WvTask, we might get here without having finished
    // the _last_ set of sure_thing streams...
    if (running_callback)
	return true;
    
    sure_thing.zap();
    
    time_t alarmleft = alarm_remaining();
    if (alarmleft == 0 && !select_ignores_buffer)
	return true; // alarm has rung
    
    oldrd = si.readable;
    oldwr = si.writable;
    oldex = si.isexception;
    oldforce = si.forceable;
    
    // when selecting on a streamlist, we always enable forceable because
    // it doesn't mean anything otherwise...
    // (or does it? maybe we shouldn't have a special case -- apenwarr)
    si.forceable = true;
    
    if (si.forceable)
    {
	if (force.readable)    si.readable = true;
	if (force.writable)    si.writable = true;
	if (force.isexception) si.isexception = true;
    }

    Iter i(*this);
    for (i.rewind(); i.next(); )
    {
	WvStream &s(*i);
	
	if (!s.isok())
	{
	    one_dead = true;
	    if (auto_prune)
		i.xunlink();
	    continue;
	}
	
	if (si.readable && !select_ignores_buffer
	    && inbuf.used() && inbuf.used() > queue_min)
	{
	    sure_thing.append(&s, false, i.link->id);
	}
	
	if (s.isok() && s.select_setup(si))
	    sure_thing.append(&s, false, i.link->id);
    }
    
    si.readable = oldrd;
    si.writable = oldwr;
    si.isexception = oldex;
    si.forceable = oldforce;
    
    return one_dead || !sure_thing.isempty();
}


bool WvStreamList::test_set(SelectInfo &si)
{
    bool one_dead = false;
    bool oldrd, oldwr, oldex, oldforce;

    oldrd = si.readable;
    oldwr = si.writable;
    oldex = si.isexception;
    oldforce = si.forceable;
    
    si.forceable = true;
    
    if (si.forceable)
    {
	if (force.readable)
	    si.readable = true;
	if (force.writable)
	    si.writable = true;
	if (force.isexception)
	    si.isexception = true;
    }
	
    Iter i(*this);
    for (i.rewind(); i.cur() && i.next(); )
    {
	WvStream &s(i);
	if (s.isok())
	{
	    if (s.test_set(si))
		sure_thing.append(&s, false);
	}
	else
	    one_dead = true;
    }
    
    si.readable = oldrd;
    si.writable = oldwr;
    si.isexception = oldex;
    si.forceable = oldforce;
    
    return one_dead || !sure_thing.isempty();
}


// distribute the callback() request to all children that select 'true'
void WvStreamList::execute()
{
    static int level = 0;
    const char *id;
    level++;
    
    WvStream::execute();
    
    TRACE("\n%*sList@%p: (%d sure) ", level, "", this, sure_thing.count());
    
    WvStreamListBase::Iter i(sure_thing);
    for (i.rewind(); i.next(); )
    {
#if STREAMTRACE
	WvStreamListBase::Iter x(*this);
	if (!x.find(&i()))
	    TRACE("Yikes! %p in sure_thing, but not in main list!\n",
		  i.cur());
#endif
	WvStream &s(*i);
	
	id = i.link->id;
	TRACE("[%p:%s]", s, id);
	
	i.xunlink();
	s.callback();
	
	// list might have changed!
	i.rewind();
    }
    
    sure_thing.zap();

    level--;
    TRACE("[DONE %p]\n", this);
}
