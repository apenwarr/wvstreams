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


bool WvStreamList::pre_select(SelectInfo &si)
{
    bool one_dead = false;
    SelectRequest oldwant;
    
    // usually because of WvTask, we might get here without having finished
    // the _last_ set of sure_thing streams...
    // 
    // FIXME: this isn't really a good fix.  It doesn't deal properly with
    // the case where a continue_selectable callback is called by someone
    // *other* than WvStreamList... eg. WvStreamClone calling its cloned
    // callback().
    // 
    // FIXME: this hack makes it so calling select() on this object from
    // its own callback always returns true.  This is why we can't apply
    // this hack inside WvStreamClone.
    if (running_callback)
	return true;
    
    sure_thing.zap();
    
    time_t alarmleft = alarm_remaining();
    if (alarmleft == 0)
	return true; // alarm has rung
    
    oldwant = si.wants;
    
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
	
	//if (si.wants.readable && inbuf.used() && inbuf.used() > queue_min)
	//    sure_thing.append(&s, false, i.link->id);
	
	if (!si.inherit_request)
	    si.wants = s.force;
	if (s.isok() && s.pre_select(si))
	    sure_thing.append(&s, false, i.link->id);
    }
    
    si.wants = oldwant;
    return one_dead || !sure_thing.isempty();
}


bool WvStreamList::post_select(SelectInfo &si)
{
    bool one_dead = false;
    SelectRequest oldwant = si.wants;
    
    Iter i(*this);
    for (i.rewind(); i.cur() && i.next(); )
    {
	WvStream &s(*i);
	if (s.isok())
	{
	    if (!si.inherit_request)
		si.wants = s.force;
	    if (s.post_select(si))
		sure_thing.append(&s, false);
	}
	else
	    one_dead = true;
    }
    
    si.wants = oldwant;
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
	
	if (s.isok())
	    s.callback();
	
	// list might have changed!
	i.rewind();
    }
    
    sure_thing.zap();

    level--;
    TRACE("[DONE %p]\n", this);
}
