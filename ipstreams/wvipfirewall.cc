/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvIPFirewall is an extremely simple hackish class that handles the Linux
 * 2.1 "ipchains" firewall.  Someday, this should be rewritten and much
 * improved.
 */
#include "wvipfirewall.h"
#include "wvinterface.h"
#include <unistd.h>


bool WvIPFirewall::enable = false;


WvIPFirewall::WvIPFirewall()
{
}


WvIPFirewall::~WvIPFirewall()
{
    zap();
}


WvString WvIPFirewall::command(const char *cmd, const WvIPPortAddr &addr)
{
    WvIPAddr ad = addr, none;
    
    return WvString("ipchains %s WvDynam -j ACCEPT -p tcp -s %s %s -d 0/0 -b",
		    cmd, ad == none ? WvString("0/0") : (WvString)ad,
		    addr.port);
}


void WvIPFirewall::add(const WvIPPortAddr &addr)
{
    addrs.append(new WvIPPortAddr(addr), true);
    WvString s(command("-I", addr));
    if (enable) system(s);
}


// note!  This does not remove the address from the list!
void WvIPFirewall::del(const WvIPPortAddr &addr)
{
    WvString s(command("-D", addr));
    if (enable) system(s);
}


void WvIPFirewall::zap()
{
    WvIPPortAddrList::Iter i(addrs);
    
    i.rewind(); i.next();
    while (i.cur())
    {
	del(i);
	i.unlink();
    }
}
