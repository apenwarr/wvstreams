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


WvString WvIPFirewall::port_command(const char *cmd, const WvIPPortAddr &addr)
{
    WvIPAddr ad(addr), none;
    
    return WvString("ipchains %s WvDynam -j ACCEPT -p tcp -s %s %s -d 0/0 -b",
		    cmd, ad == none ? WvString("0/0") : (WvString)ad,
		    addr.port);
}


WvString WvIPFirewall::redir_command(const char *cmd, const WvIPPortAddr &src,
				     int dstport)
{
    WvIPAddr ad(src), none;
    
    return WvString("ipchains %s WvRedir -j REDIRECT %s -p tcp -d %s %s",
		    cmd, dstport, 
		    ad == none ? WvString("0/0") : (WvString)ad,
		    src.port);
}


void WvIPFirewall::add_port(const WvIPPortAddr &addr)
{
    addrs.append(new WvIPPortAddr(addr), true);
    WvString s(port_command("-A", addr));
    if (enable) system(s);
}


// note!  This does not remove the address from the list, only the kernel!
void WvIPFirewall::del_port(const WvIPPortAddr &addr)
{
    WvString s(port_command("-D", addr));
    if (enable) system(s);
}


void WvIPFirewall::add_redir(const WvIPPortAddr &src, int dstport)
{
    redirs.append(new Redir(src, dstport), true);
    WvString s(redir_command("-A", src, dstport));
    if (enable) system(s);
}


void WvIPFirewall::del_redir(const WvIPPortAddr &src, int dstport)
{
    WvString s(redir_command("-D", src, dstport));
    if (enable) system(s);
}


void WvIPFirewall::zap()
{
    WvIPPortAddrList::Iter i(addrs);
    i.rewind(); i.next();
    while (i.cur())
    {
	del_port(i);
	i.unlink();
    }
    
    RedirList::Iter i2(redirs);
    i2.rewind(); i2.next();
    while (i2.cur())
    {
	del_redir(i2().src, i2().dstport);
	i2.unlink();
    }
}
