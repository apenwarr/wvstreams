/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998, 1999 Worldvisions Computer Technology, Inc.
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
	    if (i.ipaddr() != WvIPNet(ip))
	    {
		// no permission, most likely.
		index = -1;
		i.up(false);
	    }
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
	i.setipaddr(WvIPAddr());
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
    AliasList::Iter i(aliases), i_all(all_aliases);
    
    interfaces.update();
    
    for (i.rewind(); i.next(); )
    {
	assert(i_all.find(&i.data()));
	
	// the global alias entry goes down by one
	i_all.data().link_count--;
    }
    
    // empty out the local list
    for (i.rewind(), i.next(); i.cur(); )
	i.unlink();
}


WvIPAliaser::Alias *WvIPAliaser::ipsearch(WvIPAliaser::AliasList &l,
					  const WvIPAddr &ip)
{
    AliasList::Iter i(l);
    
    for (i.rewind(); i.next(); )
    {
	if (i.data().ip == WvIPAddr(ip))
	    return &i.data();
    }
    
    return NULL;
}


void WvIPAliaser::add(const WvIPAddr &ip)
{
    Alias *a;

    if (WvIPAddr(ip) == WvIPAddr() || ipsearch(aliases, ip))
	return;     // already done.
    
    a = ipsearch(all_aliases, ip);
    if (a)			    // already in global list?
    {
	// add global entry to our list and increase link count
	aliases.append(a, false);
	a->link_count++;
    }
    else if (!interfaces.islocal(ip)) // if already local, no need for alias
    {
	// create a new entry and add to both lists
        a = new Alias(ip);
	aliases.append(a, false);
	all_aliases.append(a, true);
	a->link_count++;
    }
}


void WvIPAliaser::done_edit()
{
    AliasList::Iter i(all_aliases);
    
    i.rewind(); i.next();
    while (i.cur())
    {
	Alias &a = i;
	if (!a.link_count)
	    i.unlink();
	else
	    i.next();
    } 
}


void WvIPAliaser::dump()
{
    {
	WvLog log("local aliases");
	AliasList::Iter i(aliases);
	for (i.rewind(); i.next(); )
	{
	    Alias &a = i;
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
	    Alias &a = i;
	    log("#%s = lo:wv%s: %s (%s links)\n",
		a.index, a.index, a.ip, a.link_count);
	}
	log(".\n.\n");
    }
}
