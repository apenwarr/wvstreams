/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Test for the UniConf::Iter objects.
 */
#include "uniconf.h"
#include "uniconfini.h"
#include "uniconfiter.h"
#include <wvlog.h>
#include <wvlogrcv.h>
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

template<class Iter>
void dump(WvLog &log, Iter &it)
{
    for (it.rewind(); it.next(); )
	log("  '%s' = '%s'\n", it->fullkey(), it->get());
}

int main()
{
    WvLogConsole rcv(2, WvLog::Debug4);
    WvLog log("itertest", WvLog::Info);
    UniConfRoot uniconfroot;
    UniConf h(& uniconfroot);
    h.mount(UniConfLocation("ini://test2.ini"));
    
    {
	log("Iter dump of /HTTPD:\n");
	UniConf::Iter it(h["/httpd"]);
        dump(log, it);

	log("SortedIter dump of /HTTPD:\n");
	UniConf::SortedIter sit(h["/httpd"]);
        dump(log, sit);
    }
    
    {
        UniConf r(h["/Tunnel Vision Routes"]);
	log("RecursiveIter dump of %s, Depth = ZERO:\n", r.fullkey());
	UniConf::RecursiveIter it(r, UniConfDepth::ZERO);
        dump(log, it);
        
	log("RecursiveIter dump of %s, Depth = ONE:\n", r.fullkey());
	UniConf::RecursiveIter it2(r, UniConfDepth::ONE);
        dump(log, it2);
        
	log("RecursiveIter dump of %s, Depth = CHILDREN:\n", r.fullkey());
	UniConf::RecursiveIter it3(r, UniConfDepth::CHILDREN);
        dump(log, it3);
        
	log("RecursiveIter dump of %s Depth = INFINITE:\n", r.fullkey());
	UniConf::RecursiveIter it4(r, UniConfDepth::INFINITE);
        dump(log, it4);
        
	log("RecursiveIter dump of %s Depth = DESCENDENTS:\n", r.fullkey());
	UniConf::RecursiveIter it5(r, UniConfDepth::DESCENDENTS);
        dump(log, it5);
        
	log("SortedRecursiveIter dump of %s Depth = INFINITE:\n", h.fullkey());
	UniConf::SortedRecursiveIter sit(h, UniConfDepth::INFINITE);
        dump(log, sit);
    }

    {
	log("PatternIter dump of /does_not_exist: (should be empty)\n");
	UniConf::PatternIter it(h, "/does_not_exist");
        dump(log, it);
        
	log("PatternIter dump of /httpd: (should one key)\n");
	UniConf::PatternIter it2(h, "/httpd");
        dump(log, it2);
        
	log("PatternIter dump of /*: (should be depth 1 only)\n");
	UniConf::PatternIter it3(h, "/*");
        dump(log, it3);
    }
    
    {
	log("XIter dump of /: (should be empty)\n");
	UniConf::XIter it(h, "/");
        dump(log, it);
        
	log("XIter dump of /*: (should be depth 1 only)\n");
	UniConf::XIter it2(h, "/*");
        dump(log, it2);
        
	log("XIter dump of /*/*: (should be depth 2 only)\n");
	UniConf::XIter it3(h, "/*/*");
        dump(log, it3);
        
	log("XIter dump of /httpd/*: (should be depth 2 only)\n");
	UniConf::XIter it4(h, "/httpd/*");
        dump(log, it4);
        
	log("XIter dump of /*/webmaster: (should show a few entries)\n");
	UniConf::XIter it5(h, "/*/webmaster");
        dump(log, it5);

        log("XIter dump of /*/*/monkey/*/2/*:\n");
        UniConf::XIter it6(h, "/*/*/monkey/*/2/*");
        dump(log, it6);
        
        log("SortedXIter dump of /*/*/monkey/*/2/*:\n");
        UniConf::XIter sit(h, "/*/*/monkey/*/2/*");
        dump(log, sit);
    }
    return 0;
}
