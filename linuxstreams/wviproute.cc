/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * The WvIPRoute and WvIPRouteList class, a quick (mostly hackish) attempt
 * at a way to read the Linux kernel routing table.
 */
#include "wviproute.h"
#include "wvpipe.h"
#include "wvinterface.h"
#include "wvfile.h"
#include "wvstringlist.h"

#include <net/route.h>
#include <ctype.h>




WvIPRoute::WvIPRoute(WvStringParm _ifc, const WvIPNet &_net,
		     const WvIPAddr &_gate, int _metric,
		     WvStringParm _table)
	: ifc(_ifc), ip(_net), gateway(_gate), table(_table), src()
{
    metric = _metric;
}


WvIPRoute::operator WvString() const
{
    WvIPAddr zero;
    return WvString("%s via %s %s %s metric %s%s",
		    ip, ifc, gateway, 
                    (src != zero ? WvString("src %s", src) : WvString("")),
                    metric,
		    (table != "default") 
		      ? WvString(" (table %s)", table) : WvString(""));
}


bool WvIPRoute::operator== (const WvIPRoute &r2) const
{
    return (ip.network() == r2.ip.network() && ip.netmask() == r2.ip.netmask()
	    && gateway == r2.gateway 
  	    && ifc == r2.ifc && metric == r2.metric
	    && table == r2.table);
}



/////////////////////////////////////// WvIPRouteList


WvIPRouteList::WvIPRouteList() : log("Route Table", WvLog::Debug)
{
    // nothing else to do
}


// Reads the kernel routing table, from /proc/net/route, and parses it into
// A WvIPRouteList.  Also reads the kernel 2.1.x "policy routing" tables, 
// (via the "ip" command) and parses those routes.
void WvIPRouteList::get_kernel()
{
    char *line;
    WvString ifc, table, gate, addr, mask, src;
    int metric, flags;
    bool invalid;
    WvIPRoute *r;
    WvStringList words;
    WvStringList::Iter word(words);
    
    // read each route information line from /proc/net/route; even though
    // "ip route list table all" returns all the same information plus more,
    // there's no guarantee that the ip command is available on all systems.
    WvFile kinfo("/proc/net/route", O_RDONLY);
    kinfo.getline(0);
    while ((line = kinfo.getline(0)) != NULL)
    {
	//log(WvLog::Debug2, "get_kern1: line: %s\n", line);
	
	words.zap();
	words.split(line);
	
	if (words.count() < 10)
	    continue; // weird entry
	
	word.rewind();
	word.next(); ifc   = *word;
	word.next(); addr  = *word;
	word.next(); gate  = *word;
	word.next(); flags = strtoul(*word, NULL, 16);
	word.next(); // refcnt
	word.next(); // use
	word.next(); metric = atoi(*word);
	word.next(); mask   = *word;
	
	// routes appear in the list even when not "up" -- strange.
	if (!(flags & RTF_UP))
	    continue;
	
	// the addresses in /proc/net/route are in hex.  This here is some
	// pretty sicky type-munging...
	__u32 a = strtoul(addr, NULL, 16), m = strtoul(mask, NULL, 16);
	__u32 g = strtoul(gate, NULL, 16);
	WvIPAddr aa(a), mm(m);
	WvIPNet net(aa, mm);
	WvIPAddr gw(g);
	
	r = new WvIPRoute(ifc, net, gw, metric, "default");
	append(r, true);
	//log(WvLog::Debug2, "get_kern1:  out: %s\n", *r);
    }
    
    // add more data from the kernel "policy routing" default table
    const char *argv[] = { "ip", "route", "list", "table", "all", NULL };
    WvPipe defaults(argv[0], argv, false, true, false);
    while (defaults.isok() && (line = defaults.getline(-1)) != NULL)
    {
	//log(WvLog::Debug2, "get_kern2: line: %s\n", line);
	
	invalid = false;
	ifc = gate = table = "";
	metric = 0;
	
	words.zap();
	words.split(line);
	
	if (words.count() < 3)
	    continue; // weird entry
	
	word.rewind();
	word.next();
	if (*word == "broadcast" || *word == "local")
	    continue; // these lines are weird: skip them

	WvIPNet net((*word == "default") ? WvString("0/0") : *word);
	
	while (word.next())
	{
	    WvString word1(*word);
	    if (!word.next()) break;
	    WvString word2(*word);
	    
	    if (word1 == "table")
	    {
		if (word2 == "local")
		{
		    invalid = true; // ignore 'local' table - too complex
		    break;
		}
		else
		    table = word2;
	    }
	    else if (word1 == "dev")
		ifc = word2;
	    else if (word1 == "via")
		gate = word2;
	    else if (word1 == "metric")
		metric = word2.num();
	    else if (word1 == "scope")
		; // ignore
	    else if (word1 == "proto" && word2 == "kernel")
		; // ignore
	    else if (word1 == "src")
                src = word2;
	    else
		log(WvLog::Debug, "Unknown keyvalue: '%s' '%s' in (%s)\n",
		    word1, word2, line);
	    
	    // ignore all other words - just use their defaults.
	}
	
	// if no table keyword was given, it's the default "main" table, which
	// we already read from /proc/net/route.  Skip it.
	if (!table)
	    continue;
	
	if (!ifc)
	{
	    log(WvLog::Debug2, "No interface given for this route; skipped.\n");
	    continue;
	}
	
	r = new WvIPRoute(ifc, net, gate ? WvIPAddr(gate) : WvIPAddr(),
			  metric, table);
        if (!!src)
                r->src = src;
	append(r, true);
	//log(WvLog::Debug2, "get_kern2:  out: %s\n", *r);
    }
}


static WvString realtable(WvIPRoute &r)
{
    if (!r.ip.is_default() && r.table == "default")
    	return "main";
    else
    	return r.table;
}


// we use an n-squared algorithm here, for no better reason than readability.
void WvIPRouteList::set_kernel()
{
    WvIPRouteList old_kern;
    old_kern.get_kernel();
    
    Iter oi(old_kern), ni(*this);
    
    // FIXME!!
    // Kernel 2.1.131: deleting a route with no gateway causes the kernel
    // to delete the _first_ route to that network, regardless of its
    // gateway.  This is probably to make things like "route del default"
    // more convenient.  However, it messes up if we add routes first, then
    // delete routes.
    //
    // Except for this problem, it makes more sense to add and then delete,
    // since we avoid races (we never completely remove a route to a host
    // we should be routing to).

    // delete outdated routes.
    for (oi.rewind(); oi.next(); )
    {
	if (oi->metric == 99) continue; // "magic" metric for manual override
	
	for (ni.rewind(); ni.next(); )
	    if (*ni == *oi) break;
	
	if (!ni.cur()) // hit end of list without finding a match
	{
	    WvInterface i(oi->ifc);
	    log("Del %s\n", *oi);
	    i.delroute(oi->ip, oi->gateway, oi->metric, realtable(*oi));
	}
    }

    // add any new routes.
    for (ni.rewind(); ni.next(); )
    {
	for (oi.rewind(); oi.next(); )
	    if (*oi == *ni) break;
	
	if (!oi.cur()) // hit end of list without finding a match
	{
	    WvInterface i(ni->ifc);
	    log("Add %s\n", *ni);
	    i.addroute(ni->ip, ni->gateway, ni->src, ni->metric, 
	    		realtable(*ni));
	}
    }
}


WvIPRoute *WvIPRouteList::find(const WvIPAddr &addr)
{
    Iter i(*this);
    
    for (i.rewind(); i.next(); )
    {
	if (i->ip.includes(addr))
	    return &i();
    }
    
    return NULL;
}
