/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * The WvIPRoute and WvIPRouteList class, a quick (mostly hackish) attempt
 * at a way to read the Linux kernel routing table.
 */
#include "wviproute.h"
#include "wvstream.h"
#include "wvinterface.h"
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



/////////////////////////////////////// WvIPRouteList


WvIPRouteList::WvIPRouteList() : log("Route Table", WvLog::Debug)
{
    // nothing else to do
}


void WvIPRouteList::get_kernel()
{
    WvFile kinfo("/proc/net/route", O_RDONLY);
    char *line, *addr, *gate, *mask, *flags, *metric, *end;
    bool last_white;
    
    // skip header
    kinfo.getline(0);

    // read each route information line
    last_white = false;
    while ((line = kinfo.getline(0)) != NULL)
    {
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
	
	append(new WvIPRoute(line, addr, mask, gate, atoi(metric)), true);
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
