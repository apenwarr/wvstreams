/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvIPAliaser handles IP aliasing in the Linux kernel.  See wvipaliaser.h.
 */
#include "wvipaliaser.h"
#include "wvinterface.h"
#include <assert.h>


WvIPAliaser::AliasList WvIPAliaser::all_aliases;



///////////////////////////////////// WvIPAliaser::Alias



WvIPAliaser::Alias::Alias(const WvIPAddr &_ip) : ip(_ip)
{
    WvIPAddr noip;
    WvIPNet nonet(noip, noip);
    link_count = 0;
    
    for (index = 0; index < 256; index++)
    {
	WvInterface i(WvString("lo:wv%s", index));
	
	if (!i.isup() || i.ipaddr() == nonet) // not in use yet!
	{
	    i.setipaddr(ip);
	    i.up(true);
	    if (WvIPAddr(i.ipaddr()) != ip)
	    {
		// no permission, most likely.
		index = -1;
		i.up(false);
	    }
	    return;
	}
	
	if (i.isup() && WvIPNet(i.ipaddr(),32) == ip)
	{
	    // a bit weird... this alias already has the right address.
	    // Keep it.
	    return; 
	}
    }
    
    // got through all possible names without a free one?  Weird!
    index = -1;
}


WvIPAliaser::Alias::~Alias()
{
    if (index >= 0)
    {
	WvInterface i(WvString("lo:wv%s", index));
	// i.setipaddr(WvIPAddr()); // not necessary in recent kernels
	i.up(false);
    }
}



//////////////////////////////////// WvIPAliaser



WvIPAliaser::WvIPAliaser() : interfaces()
{
    // nothing to do
}


WvIPAliaser::~WvIPAliaser()
{
    // clear the alias list
    start_edit();
    done_edit();
}


void WvIPAliaser::start_edit()
{
    AliasList::Iter i(aliases);
    
    #ifndef NDEBUG
    AliasList::Iter i_all(all_aliases);
    #endif
    
    interfaces.update();
    
    for (i.rewind(); i.next(); )
    {
	assert(i_all.find(i.ptr()));
	
	// the global alias entry goes down by one
	i().link_count--;
    }
    
    // empty out the local list
    aliases.zap();
}


WvIPAliaser::Alias *WvIPAliaser::ipsearch(WvIPAliaser::AliasList &l,
					  const WvIPAddr &ip)
{
    AliasList::Iter i(l);
    
    for (i.rewind(); i.next(); )
    {
	if (i->ip == WvIPAddr(ip))
	    return i.ptr();
    }
    
    return NULL;
}


bool WvIPAliaser::add(const WvIPAddr &ip)
{
    Alias *a;

    if (WvIPAddr(ip) == WvIPAddr() || ipsearch(aliases, ip))
	return false;     // already done.

    // If the alias is already a local address, there is no need for an alias.
    // We have to be careful that we don't find an existing alias as the
    // local interface.  Otherwise, we'll toggle that alias on and off.
    WvString ifc(interfaces.islocal(WvIPAddr(ip)));
    if (!!ifc && !strchr(ifc, ':')) // Make sure it is a real interface
	return false;

    a = ipsearch(all_aliases, ip);
    if (a)
    {
	// It's already in the global list, so we add its entry to
	// our list and increase the link count.
	aliases.append(a, false);
	a->link_count++;
	return false;
    }
    else
    {
	// It's not there, so we add a new alias to the global list and
	// our local list.
	a = new Alias(ip);
	aliases.append(a, false);
	all_aliases.append(a, true);
	a->link_count++;
	return true;
    }
}


bool WvIPAliaser::done_edit()
{
    bool any_change=false;
    AliasList::Iter i(all_aliases);
    
    i.rewind(); i.next();
    while (i.cur())
    {
	Alias &a = *i;
	if (!a.link_count) {
	    i.unlink();
	    any_change = true;
	} else
	    i.next();
    } 

    return any_change;
}


void WvIPAliaser::dump()
{
    {
	WvLog log("local aliases");
	AliasList::Iter i(aliases);
	for (i.rewind(); i.next(); )
	{
	    Alias &a = *i;
	    log("#%s = lo:wv%s: %s (%s links)\n",
		a.index, a.index, a.ip, a.link_count);
	}
	log(".\n");
    }

    {
	WvLog log("global aliases");
	AliasList::Iter i(all_aliases);
	for (i.rewind(); i.next(); )
	{
	    Alias &a = *i;
	    log("#%s = lo:wv%s: %s (%s links)\n",
		a.index, a.index, a.ip, a.link_count);
	}
	log(".\n.\n");
    }
}
