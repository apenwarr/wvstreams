/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
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
    return !s ? 0 : WvHash((const char *)s);
}

// FIXME: does this suck?
unsigned WvHash(const int &i)
{
    return i;
}


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
WvLink *WvHashTableBase::prevlink(WvListBase *slots, const void *data,
			      unsigned hash, Comparator *comp)
{
    WvListBase::IterBase i(slots[hash % numslots]);
    WvLink *prev;
    
    i.rewind();
    for (prev = i.cur(); prev->next; prev = i.next())
    {
	if (comp(data, prev->next->data))
	    break;
    }
    return prev;
}


void *WvHashTableBase::genfind(WvListBase *slots, const void *data,
			      unsigned hash, Comparator *comp)
{
    WvLink *prev = prevlink(slots, data, hash, comp);
    if (prev->next)
	return prev->next->data;
    else
	return NULL;
}


size_t WvHashTableBase::count() const
{
    size_t count = 0;
    
    for (unsigned i = 0; i < numslots; i++)
	count += slots[i].count();
    return count;
}


WvLink *WvHashTableBase::IterBase::next()
{
    link = link->next;
    while (!link && tblindex < tbl->numslots - 1)
	link = tbl->slots[++tblindex].head.next;
    return link;
}


static WvHashTableBase::SorterBase::CompareFunc *actual_compare = NULL;

static int magic_compare(const void *_a, const void *_b)
{
    WvLink *a = *(WvLink **)_a, *b = *(WvLink **)_b;
    return actual_compare(a->data, b->data);
}


// Note that this is largely the same as WvLink::SorterBase::rewind(),
// except we iterate through a bunch of lists instead of a single one.
void WvHashTableBase::SorterBase::rewind(CompareFunc *cmp)
{
    if (array)
        delete array;
    array = lptr = NULL;

    int n = tbl->count();
    array = new WvLink * [n+1];
    WvLink **aptr = array;

    // fill the array with data pointers for sorting, so that the user doesn't
    // have to deal with the WvLink objects.  Put the WvLink pointers back 
    // in after sorting.
    IterBase i(*tbl);
    aptr = array;
    for (i.rewind(); i.next(); )
    {
        *aptr = i.cur();
        aptr++;
    }
    
    *aptr = NULL;

    // sort the array.  "Very nearly re-entrant" (unless the compare function
    // ends up being called recursively or something really weird...)
    CompareFunc *old_compare = actual_compare;
    actual_compare = cmp;
    qsort(array, n, sizeof(WvLink *), magic_compare);
    actual_compare = old_compare;

    lptr = NULL;    // subsequent next() will set it to first element.
}
