/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Test for the UniConf::Iter objects.
 */
#include "uniconfini.h"
#include "uniconfiter.h"
#include <assert.h>


#if 0
static int hconfcmp(const UniConf *a, const UniConf *b)
{
    return a->key() == b->key();
}


static int rhconfcmp(const UniConf *a, const UniConf *b)
{
    return a->full_key() == b->full_key();
}
#endif


int main()
{
    WvLog log("itertest", WvLog::Info);
    UniConf h;
    h.mount(UniConfLocation("ini://test2.ini"));
    
    {
	log("Non-recursive dump of /HTTPD:\n");
	UniConf::Iter i(h["/httpd"]);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), i->value());
    }
    
    {
	log("Recursive dump:\n");
	UniConf::RecursiveIter i(h);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), i->value());
    }
    
#if 0
    {
	log("Sorted non-recursive dump of /HTTPD:\n");
	UniConf::Sorter i(h["/HTTPD"], hconfcmp);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), i->value());
    }
    
    {
	log("Sorted recursive dump:\n");
	UniConf::RecursiveSorter i(h, rhconfcmp);
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), i->value());
    }
    
    {
	log("Extended iter:\n");
	UniConf::XIter i(h, "*/*/monkey/*/2/*");
	for (i.rewind(); i.next(); )
	    log("  '%s' = '%s'\n", i->full_key(), i->value());
    }
#endif
    return 0;
}
