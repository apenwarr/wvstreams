/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * Small, efficient, type-safe hash table class.  See wvhashtable.h.
 */
#include "wvhashtable.h"
#include "wvstring.h"

// Note: this hash function is case-insensitive since it ignores the
// bit in ASCII that defines case.  You may want to take advantage of this.
unsigned WvHash(const char *s)
{
    unsigned hash = 0, slide, andval;
    if (!s) return 0;
    
    slide = sizeof(hash)*8 - 5;
    andval = 0x1F << slide;
    
    while (*s)
	hash = (hash<<5) ^ (*(s++) & 0x1F) ^ ((hash & andval) >> slide);
    
    return hash;
}

unsigned WvHash(const WvString &s)
{
    return !s ? 0 : WvHash((char *)s);
}


// we do not accept the _numslots value directly.  Instead, we find the
// next number of slots which is >= _numslots and one less then a power
// of 2.  This usually results in a fairly good hash table size.
WvHashTable::WvHashTable(unsigned _numslots)
{
    int slides = 1;
    while ((_numslots >>= 1) != 0)
	slides++;
    numslots = (1 << slides) - 1;
}


// never returns NULL.  If the object is not found, the 'previous' link
// is the last one in the list.
WvLink *WvHashTable::prevlink(WvList *slots, const void *data,
			      unsigned hash, Comparator *comp)
{
    WvList::IterBase i(slots[hash % numslots]);
    WvLink *prev;
    
    i.rewind();
    for (prev = i.cur(); prev->next; prev = i.next())
    {
	if (comp(data, prev->next->data))
	    break;
    }
    return prev;
}


void *WvHashTable::genfind(WvList *slots, const void *data,
			      unsigned hash, Comparator *comp)
{
    WvLink *prev = prevlink(slots, data, hash, comp);
    if (prev->next)
	return prev->next->data;
    else
	return NULL;
}


size_t WvHashTable::count() const
{
    size_t count = 0;
    
    for (unsigned i = 0; i < numslots; i++)
	count += slots[i].count();
    return count;
}


WvLink *WvHashTable::IterBase::next()
{
    link = link->next;
    while (!link && tblindex < tbl->numslots - 1)
	link = tbl->slots[++tblindex].head.next;
    return link;
}
