/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Test for the UniConf::Iter objects.
 */
#include "uniconfini.h"
#include "uniconfiter.h"
#include <assert.h>


static int hconfcmp(const UniConf *a, const UniConf *b)
{
    return a->name == b->name;
}


static int rhconfcmp(const UniConf *a, const UniConf *b)
{
    return a->full_key() == b->full_key();
}


int main()
{
    WvLog log("itertest", WvLog::Info);
    UniConf h;
    h.mount(new UniConfIniFile(&h, "test2.ini"));
    
    {
	// make sure iterators don't create the 'children' array
	UniConf::Iter i(h["/spam/spam/spam"]);
	for (i.rewind(); i.next(); )
	    *i;
	assert(!h["spam/spam/spam"].has_children());
    }
    
    {
	log("Non-recursive dump of /HTTPD:\n");
	UniConf::Iter i(h["/httpd"]);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    {
	log("Recursive dump:\n");
	UniConf::RecursiveIter i(h);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    {
	log("Sorted non-recursive dump of /HTTPD:\n");
	UniConf::Sorter i(h["/HTTPD"], hconfcmp);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    {
	log("Sorted recursive dump:\n");
	UniConf::RecursiveSorter i(h, rhconfcmp);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    {
	log("Extended iter:\n");
	UniConf::XIter i(h, "*/*/monkey/*/2/*");
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), *i);
    }
    
    return 0;
}
