/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Test for the WvHConf::Iter object.
 */
#include "wvhconfini.h"
#include "wvhconfiter.h"
#include <assert.h>


static int hconfcmp(const WvHConf *a, const WvHConf *b)
{
    return strcasecmp(a->name, b->name);
}


static int rhconfcmp(const WvHConf *a, const WvHConf *b)
{
    return strcasecmp(a->full_key().printable(),
		      b->full_key().printable());
}


int main()
{
    WvLog log("itertest", WvLog::Info);
    WvHConf h;
    h.generator = new WvHConfIniFile(&h, "test2.ini");
    h.load();
    
    {
	// make sure iterators don't create the 'children' array
	WvHConf::Iter i(h["/spam/spam/spam"]);
	for (i.rewind(); i.next(); )
	    *i;
	assert(!h["spam/spam/spam"].has_children());
    }
    
    {
	log("Non-recursive dump of /HTTPD:\n");
	WvHConf::Iter i(h["/httpd"]);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    {
	log("Recursive dump:\n");
	WvHConf::RecursiveIter i(h);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    {
	log("Sorted non-recursive dump of /HTTPD:\n");
	WvHConf::Sorter i(h["/HTTPD"], hconfcmp);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    {
	log("Sorted recursive dump:\n");
	WvHConf::RecursiveSorter i(h, rhconfcmp);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    {
	log("Extended iter:\n");
	WvHConf::XIter i(h, "*/*/monkey/*/2/*");
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    return 0;
}
