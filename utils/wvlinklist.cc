/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * Implementation of a Linked List management class, or rather, macros that
 * declare arbitrary linked list management classes.
 * 
 * wvlinklist.h does all the real work.
 */
#include "wvlinklist.h"

WvLink::WvLink(void *_data, WvLink *prev, WvLink *&tail, bool _auto_free)
{
    data = _data;
    next = prev->next;
    if (!next) tail = this;
    prev->next = this;
    auto_free = (unsigned)_auto_free;
}


size_t WvList::count() const
{
    WvLink *l;
    size_t n = 0;
    
    for (l = head.next; l; l = l->next)
	n++;
    return n;
}


WvLink *WvList::IterBase::find(const void *data)
{
    for (rewind(); next(); )
    {
	if (link->data == data)
	    break;
    }
    
    return link;
}
