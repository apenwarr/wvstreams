/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 * 
 * WvIPFirewall is an extremely simple hackish class that handles the Linux
 * 2.1/2.2 "ipchains" firewall.  It's okay to create more than one instance
 * of this class; they'll co-operate.
 * 
 * They need you to have created WvDynam and WvRedir chains already,
 * however, and call them from the right places in the Input and/or Forward
 * firewalls.
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


WvString WvIPFirewall::port_command(const char *cmd, const char *proto,
				    const WvIPPortAddr &addr)
{
    WvIPAddr ad(addr), none;
    
    return WvString("ipchains %s WvDynam -j ACCEPT -p %s -d %s %s",
		    cmd, proto, ad == none ? WvString("0/0") : (WvString)ad,
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
    WvString s(port_command("-A", "tcp", addr)),
    	    s2(port_command("-A", "udp", addr));
    if (enable)
    {
	system(s);
	system(s2);
    }
}


// note!  This does not remove the address from the list, only the kernel!
void WvIPFirewall::del_port(const WvIPPortAddr &addr)
{
    WvString s(port_command("-D", "tcp", addr)),
    	    s2(port_command("-D", "udp", addr));
    if (enable)
    {
	system(s);
	system(s2);
    }
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
