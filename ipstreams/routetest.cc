/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-1999 Worldvisions Computer Technology, Inc.
 *
 * WvIPRoute test.  Gets the kernel routing table, adds some new routes, and
 * writes the table.
 *
 */

#include "wviproute.h"
#include "wvlog.h"

int main()
{
    WvLog l("test");
    WvIPRouteList r;
    WvIPRouteList::Iter i(r);
    WvIPRoute *rr;
    
    r.get_kernel();
    
    for (i.rewind(); i.next(); )
	l("%s\n", i());
    
    WvIPAddr a("192.168.42.22");
    rr = r.find(a);
    if (rr)
	l("\n%s through:\n  %s\n", a, *r.find(a));

    WvIPAddr b("1.2.3.4");
    rr = r.find(b);
    if (rr)
	l("\n%s through:\n  %s\n", b, *r.find(b));
    
    l("Check point.\n");
    
    WvIPRouteList r2;
    
    r2.append(new WvIPRoute("eth1", WvIPNet("24.112.104.0/255.255.252.0"),
			    "0", 5), true);
    r2.append(new WvIPRoute("eth0", WvIPNet("192.168.42.4/24"),
			    "0", 0), true);
    r2.append(new WvIPRoute("eth1", WvIPNet("0.0.0.0/0"),
			    "24.112.104.1", 1), true);
    r2.set_kernel();
}
