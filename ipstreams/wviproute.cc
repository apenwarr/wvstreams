/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * The WvIPRoute and WvIPRouteList class, a quick (mostly hackish) attempt
 * at a way to read the Linux kernel routing table.
 */
#include "wviproute.h"
#include "wvpipe.h"
#include "wvinterface.h"
#include "wvfile.h"

#include <net/route.h>
#include <ctype.h>




//////////////////////////////////////// WvIPRoute



WvIPRoute::WvIPRoute(const WvString &_ifc, const WvString &_addr,
		     const WvString &_mask,const WvString &_gate, 
		     int _metric)
{
    __u32 addr, mask, gate;
    
    ifc = _ifc;
    ifc.unique();
    
    addr = strtoul(_addr, NULL, 16);
    mask = strtoul(_mask, NULL, 16);
    ip = WvIPNet(WvIPAddr(addr), WvIPAddr(mask));
    
    gate = strtoul(_gate, NULL, 16);
    gateway = WvIPAddr(gate);
    
    metric = _metric;
}


WvIPRoute::WvIPRoute(const WvString &_ifc, const WvIPNet &_net,
		     const WvIPAddr &_gate, int _metric)
	: ifc(_ifc), ip(_net), gateway(_gate)
{
    ifc.unique();
    metric = _metric;
}


WvIPRoute::operator WvString() const
{
    return WvString("%s via %s %s metric %s",
		    ip, ifc, gateway, metric);
}


bool WvIPRoute::operator== (const WvIPRoute &r2) const
{
    return (ip.network() == r2.ip.network() && ip.netmask() == r2.ip.netmask()
	    && gateway == r2.gateway 
  	    && ifc == r2.ifc && metric == r2.metric);
}


static char *find_space(char *str)
{
    while (str && *str && !isspace(*str))
	str++;
    return str;
}


static char *find_nonspace(char *str)
{
    while (str && *str && isspace(*str))
	str++;
    return str;
}


static char *next_col(char *str)
{
    return find_nonspace(find_space(str));
}


static void nullify(char *str)
{
    *find_space(str) = 0;
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
    WvFile kinfo("/proc/net/route", O_RDONLY);
    char *line, *ifc, *addr, *gate, *mask, *flags, *metric, *end;
    char *keyword, *value;
    bool last_white;
    
    // skip header
    kinfo.getline(0);

    // read each route information line
    last_white = false;
    while ((line = kinfo.getline(0)) != NULL)
    {
	ifc = line;
	addr = next_col(line);
	gate = next_col(addr);
	flags = next_col(gate);
	metric = next_col(next_col(next_col(flags)));
	mask = next_col(metric);

	// routes appear in the list even when not "up" -- strange.
	if (! (strtoul(flags, NULL, 16) & RTF_UP))
	    continue;
	
	// null-terminate interface name
	end = find_space(line);
	*end = 0;
	
	append(new WvIPRoute(ifc, addr, mask, gate, atoi(metric)), true);
    }
    
    
    // append data from the 2.1.x kernel "policy routing" default table
    const char *argv[] = { "ip", "route", "list", "table", "default", NULL };
    WvPipe defaults(argv[0], argv, false, true, false);
    while (defaults.isok() && (line = defaults.getline(-1)) != NULL)
    {
	// log(WvLog::Debug2, "iproute: %s\n", line);
	
	keyword = line;
	line = next_col(keyword);
	nullify(keyword);
	
	if (strcmp(keyword, "default"))
	{
	    log(WvLog::Debug, "skipping unknown route '%s'\n", keyword);
	    continue;
	}
	
	ifc = addr = gate = flags = metric = mask = NULL;
	
	do
	{
	    keyword = line;
	    value = next_col(keyword);
	    line = next_col(value);
	    nullify(keyword);
	    nullify(value);
	    
	    if (!strcmp(keyword, "via"))
		gate = value;
	    else if (!strcmp(keyword, "dev"))
		ifc = value;
	    else if (!strcmp(keyword, "metric"))
		metric = value;
	    else if (!strcmp(keyword, "scope"))
		; // ignore
	    else
		log(WvLog::Debug, "Unknown keyvalue: '%s' '%s'\n",
		    keyword, value);
	} while (*line);
	
	if (!ifc)
	{
	    log(WvLog::Debug2, "No interface given for this route; skipped.");
	    continue;
	}
	
	append(new WvIPRoute(ifc, WvIPNet("0.0.0.0", 0),
			     gate ? WvIPAddr(gate) : WvIPAddr(),
			     metric ? atoi(metric) : 0), true);
    }
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
	if (oi().metric == 99) continue; // "magic" metric for manual override
	
	for (ni.rewind(); ni.next(); )
	    if (ni() == oi()) break;
	
	if (!ni.cur()) // hit end of list without finding a match
	{
	    WvInterface i(oi().ifc);
	    log("Del %s\n", oi());
	    i.delroute(oi().ip, oi().gateway, oi().metric);
	}
    }

    // add any new routes.
    for (ni.rewind(); ni.next(); )
    {
	for (oi.rewind(); oi.next(); )
	    if (oi() == ni()) break;
	
	if (!oi.cur()) // hit end of list without finding a match
	{
	    WvInterface i(ni().ifc);
	    log("Add %s\n", ni());
	    i.addroute(ni().ip, ni().gateway, ni().metric);
	}
    }
}


WvIPRoute *WvIPRouteList::find(const WvIPAddr &addr)
{
    Iter i(*this);
    
    for (i.rewind(); i.next(); )
    {
	if (i.data().ip.includes(addr))
	    return &i.data();
    }
    
    return NULL;
}
