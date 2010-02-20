/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#include "wvhttppool.h"
#include "wvfile.h"
#include "strutils.h"
#include <signal.h>

static bool want_to_die = false;

int main(int argc, char **argv)
{
    WvLog log("posttest", WvLog::Info);
    WvIStreamList l;
    WvHttpPool p;
    WvString headers("");
    
    l.append(&p, false, "WvHttpPool");
    
    if (argc != 3)
    {
	wverr->print("usage: %s <url> <filename>\n", argv[0]);
	return 99;
    }
    WvString url(argv[1]), infile(argv[2]);
    
    p.addurl(url, "POST", "", new WvFile(infile, O_RDONLY));
    
    while (!want_to_die && p.isok() && !p.idle())
    {
	fprintf(stderr, "."); fflush(stderr);
	if (l.select(1000))
	{
	    fprintf(stderr, "!"); fflush(stderr);
	    l.runonce(-1);
	}
    }
    
    if (!p.isok() && p.geterr())
	log("HttpPool: %s\n", p.errstr());
    
    return 0;
}
