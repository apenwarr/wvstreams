/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 *
 * Definition for the WvStringCache class.  See wvstringcache.h.
 */
#include "wvstringcache.h"
#include "wvstringlist.h"


WvStringTable *WvStringCache::t;
int WvStringCache::refcount;


WvStringCache::WvStringCache()
{
    refcount++;
    if (!t)
	t = new WvStringTable;
}


WvStringCache::~WvStringCache()
{
    refcount--;
    if (!refcount)
    {
	delete t;
	t = NULL;
    }
    else
	clean();
}
    

WvString WvStringCache::get(WvStringParm s)
{
    // return s; // disable cache
    WvString *ret = (*t)[s];
    if (ret)
    {
	// printf("found(%s)\n", s.cstr());
	return *ret;
    }
    else
    {
	// printf("  new(%s)\n", s.cstr());
	ret = new WvString(s);
	t->add(ret, true);
	return *ret;
    }
}


void WvStringCache::clean()
{
    WvStringList l;
    
    // use a two-stage process so the iterator doesn't get messed up
    {
	WvStringTable::Iter i(*t);
	for (i.rewind(); i.next(); )
	{
	    if (i->is_unique()) // last remaining instance
	    {
		// printf("CLEANUP(%s)\n", i->cstr());
		l.append(i.ptr(), false);
	    }
	}
    }
    
    {
	WvStringList::Iter i(l);
	for (i.rewind(); i.next(); )
	    t->remove(i.ptr());
    }
    
    // printf("CLEANUP: %d elements left.\n", (int)t.count());
}


