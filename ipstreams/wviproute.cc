/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * The WvIPRoute and WvIPRouteList class, a quick (mostly hackish) attempt
 * at a way to read the Linux kernel routing table.
 */
#include "wviproute.h"
#include "wvstream.h"
#include <net/route.h>
#include <ctype.h>




//////////////////////////////////////// WvIPRoute



WvIPRoute::WvIPRoute(const char *_ifc, const char *_addr, const char *_mask,
		     const char *_gate)
{
    __u32 addr, mask, gate;
    
    ifc = _ifc;
    
    addr = strtoul(_addr, NULL, 16);
    mask = strtoul(_mask, NULL, 16);
    ip = WvIPNet(WvIPAddr(addr), WvIPAddr(mask));
    
    gate = strtoul(_gate, NULL, 16);
    gateway = WvIPAddr(gate);
}


WvIPRoute::operator WvString() const
{
    return WvString("%s via %s on interface %s", ip, gateway, ifc);
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



void WvIPRouteList::setup()
{
    WvFile kinfo("/proc/net/route", O_RDONLY);
    char *line, *end, *addr, *mask, *gate, *flags;
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
	mask = next_col(next_col(next_col(next_col(flags))));

	// routes appear even when not "up" -- strange.
	if (! (strtoul(flags, NULL, 16) & RTF_UP))
	    continue;
	
	// null-terminate interface name
	end = find_space(line);
	*end = 0;
	
	append(new WvIPRoute(line, addr, mask, gate), true);
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
