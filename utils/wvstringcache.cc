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
size_t WvStringCache::clean_threshold;

WvStringCache::WvStringCache()
{
    refcount++;
    if (!t)
    {
	t = new WvStringTable;
	clean_threshold = 0;
    }
}


WvStringCache::~WvStringCache()
{
    refcount--;
    if (!refcount)
    {
	delete t;
	t = NULL;
	clean_threshold = 0;
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
    // do we actually need to clean yet?  Skip it if we haven't added too
    // many items since the last clean, since cleaning is pretty slow.
    if (t->count() < clean_threshold)
	return;
    
    WvStringList l;
    
    // use a two-stage process so the iterator doesn't get messed up
    // FIXME: this might actually be unnecessary with WvScatterHash, but
    // someone should actually confirm that before taking this out.
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
    
//    printf("CLEANUP-1: %d elements at start (%d to remove)\n",
//	   (int)t->count(), (int)l.count());
    
    {
	WvStringList::Iter i(l);
	for (i.rewind(); i.next(); )
	    t->remove(i.ptr());
    }
    
    clean_threshold = t->count() + t->count()/10 + 1;
    
//    printf("CLEANUP-2: %d elements left (thres=%d).\n", 
//	   (int)t->count(), (int)clean_threshold);
}


