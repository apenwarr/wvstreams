/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Test for the UniConf::Iter objects.
 */
#include "uniconfroot.h"
#include "uniinigen.h"
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
void dump(WvLog &log, Iter &i)
{
    for (i.rewind(); i.next(); )
	log("  '%s' = '%s'\n", i->fullkey(), i->getme());
}

int main()
{
    WvLogConsole rcv(2, WvLog::Debug4);
    WvLog log("itertest", WvLog::Info);
    UniConfRoot uniconfroot("ini:test2.ini");
    UniConf h(uniconfroot);
    
    {
	log("Iter dump of /HTTPD:\n");
	UniConf::Iter i(h["/httpd"]);
        dump(log, i);

	log("SortedIter dump of /HTTPD:\n");
	UniConf::SortedIter si(h["/httpd"]);
        dump(log, si);
    }
    
    {
        UniConf r(h["/Tunnel Vision Routes"]);
	log("RecursiveIter dump of %s:\n", r.fullkey());
	UniConf::RecursiveIter i(r);
        dump(log, i);
    }

    {
	log("XIter dump of /: (should be one key)\n");
	UniConf::XIter i(h, "/");
        dump(log, i);
        
	log("XIter dump of /*: (should be depth 1 only)\n");
	UniConf::XIter i2(h, "/*");
        dump(log, i2);
        
	log("XIter dump of /*/*: (should be depth 2 only)\n");
	UniConf::XIter i3(h, "/*/*");
        dump(log, i3);
        
	log("XIter dump of /httpd/*: (should be depth 2 only)\n");
	UniConf::XIter i4(h, "/httpd/*");
        dump(log, i4);
        
	log("XIter dump of /*/webmaster: (should show a few entries)\n");
	UniConf::XIter i5(h, "/*/webmaster");
        dump(log, i5);

        log("XIter dump of /*/*/monkey/*/2/*:\n");
        UniConf::XIter i6(h, "/*/*/monkey/*/2/*");
        dump(log, i6);
        
        log("XIter dump of /.../2/*:\n");
        UniConf::XIter i7(h, "/.../2/*");
        dump(log, i7);
        
        log("XIter dump of /.../*/4/...:\n");
        UniConf::XIter i8(h, "/.../*/4/...");
        dump(log, i8);
        
        log("XIter dump of /.../virus scanner/...:\n");
        UniConf::XIter i9(h, "/.../virus scanner/...");
        dump(log, i9);
        
        log("SortedXIter dump of /*/*/monkey/*/2/*:\n");
        UniConf::XIter si(h, "/*/*/monkey/*/2/*");
        dump(log, si);
    }
    return 0;
}
