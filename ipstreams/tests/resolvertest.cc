/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvResolver test program.  Tries to look up two host names given on the
 * command line.
 */
#include "wvresolver.h"
#include "wvlog.h"

void test(WvResolver &dns, int argc, char **argv)
{
    WvLog log("resolvertest", WvLog::Info);
    const WvIPAddr *addr;
    int res1, res2;
    const char *n1 = argc > 1 ? argv[1] : "www";
    const char *n2 = argc > 2 ? argv[2] : "www.google.com";
    
    log("Using [1]='%s', [2]='%s'\n", n1, n2);
    
    res1 = res2 = -1;

    while (res1 < 0 || res2 < 0)
    {
	if (res1 < 0)
	{
	    res1 = dns.findaddr(100, n1, &addr);
	    if (res1 > 0)
		log.print("Found address for 1: %s\n", *addr);
	    else if (res1 < 0)
		log.print("[1] ");
	    else
		log(WvLog::Error, "1 not in DNS.\n");
	}

	if (res2 < 0)
	{
	    res2 = dns.findaddr(100, n2, &addr);
	    if (res2 > 0)
		log.print("Found address for 2: %s\n", *addr);
	    else if (res2 < 0)
		log.print("[2] ");
	    else
		log(WvLog::Error, "2 not in DNS.\n");
	}
    }
    
}

int main(int argc, char **argv)
{
    {
	WvResolver dns;
	test(dns, argc, argv);
	test(dns, argc, argv);
    }

    {
	WvResolver dns;
	test(dns, argc, argv);
    }
    
    return 0;
}
