/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Small, efficient, type-safe hash table class.  See wvhashtable.h.
 */
#include "wvhashtable.h"
#include "wvstring.h"

// we do not accept the _numslots value directly.  Instead, we find the
// next number of slots which is >= _numslots and one less then a power
// of 2.  This usually results in a fairly good hash table size.
WvHashTableBase::WvHashTableBase(unsigned _numslots)
{
    int slides = 1;
    while ((_numslots >>= 1) != 0)
	slides++;
    numslots = (1 << slides) - 1;
}


// never returns NULL.  If the object is not found, the 'previous' link
// is the last one in the list.
WvLink *WvHashTableBase::prevlink(WvListBase *wvslots, const void *data,
			      unsigned hash) const
{
    WvListBase::IterBase i(wvslots[hash % numslots]);
    WvLink *prev;
    
    i.rewind();
    for (prev = i.cur(); prev->next; prev = i.next())
    {
	if (compare(data, prev->next->data))
	    break;
    }
    return prev;
}


void *WvHashTableBase::genfind(WvListBase *wvslots, const void *data,
			      unsigned hash) const
{
    WvLink *prev = prevlink(wvslots, data, hash);
    if (prev->next)
	return prev->next->data;
    else
	return NULL;
}


size_t WvHashTableBase::count() const
{
    size_t count = 0;
    
    for (unsigned i = 0; i < numslots; i++)
	count += wvslots[i].count();
    return count;
}


bool WvHashTableBase::isempty() const
{
    for (unsigned i = 0; i < numslots; i++)
        if (! wvslots[i].isempty())
            return false;
    return true;
}


WvLink *WvHashTableBase::IterBase::next()
{
    link = link->next;
    while (!link && tblindex < tbl->numslots - 1)
	link = tbl->wvslots[++tblindex].head.next;
    return link;
}
