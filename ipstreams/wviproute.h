/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * The WvIPRoute and WvIPRouteList class, a quick (mostly hackish) attempt
 * at a way to read the Linux kernel routing table.
 */
#ifndef __WVIPROUTE_H
#define __WVIPROUTE_H

#include "wvaddr.h"
#include "wvlinklist.h"


class WvIPRoute
{
public:
    WvIPRoute(const WvString &_ifc, const WvString &_addr,
	      const WvString &_mask,const WvString &_gate);
    operator WvString() const;
    
    WvString ifc;
    WvIPNet ip;
    WvIPAddr gateway;
};


DeclareWvList2(WvIPRoute,
	       // automatically fill the list with appropriate data
	       void setup();
	       
	       // find the routing entry that matches 'addr'
	       WvIPRoute *find(const WvIPAddr &addr);
	       );


#endif // __WVIPROUTE_H
