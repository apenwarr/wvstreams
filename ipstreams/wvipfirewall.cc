/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
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


bool WvIPFirewall::enable = false, WvIPFirewall::ignore_errors = true;


WvIPFirewall::WvIPFirewall()
{
    system(WvString("iptables -F Services %s", shutup()));
    system(WvString("iptables -t nat -F TProxy %s", shutup()));
}


WvIPFirewall::~WvIPFirewall()
{
    zap();
}


WvString WvIPFirewall::port_command(const char *cmd, const char *proto,
				    const WvIPPortAddr &addr)
{
    WvIPAddr ad(addr), none;
    
    return WvString("iptables %s Services -j ACCEPT -p %s "
		    "%s --dport %s "
		    "%s",
		    cmd, proto,
		    ad == none ? WvString("") : WvString("-d %s", ad),
		    addr.port,
		    shutup());
}


WvString WvIPFirewall::redir_command(const char *cmd, const WvIPPortAddr &src,
				     int dstport)
{
    WvIPAddr ad(src), none;
    
    return WvString("iptables -t nat %s TProxy "
		    "-p tcp %s --dport %s "
		    "-j REDIRECT --to-ports %s "
		    "%s",
		    cmd,
		    ad == none ? WvString("") : WvString("-d %s", ad),
		    src.port, dstport,
		    shutup());
}


WvString WvIPFirewall::proto_command(const char *cmd, const char *proto)
{
    return WvString("iptables %s Services -p %s -j ACCEPT "
		    "%s",
		    cmd, proto, shutup());
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
    WvIPPortAddrList::Iter i(addrs);
    for (i.rewind(); i.next(); )
    {
	if (*i == addr)
	{
	    WvString s(port_command("-D", "tcp", addr)),
		    s2(port_command("-D", "udp", addr));
	    if (enable)
	    {
		system(s);
		system(s2);
	    }
	    return;
	}
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
    RedirList::Iter i(redirs);
    for (i.rewind(); i.next(); )
    {
	if (i->src == src && i->dstport == dstport)
	{
	    WvString s(redir_command("-D", src, dstport));
	    if (enable) system(s);
	    return;
	}
    }
}


void WvIPFirewall::add_proto(WvStringParm proto)
{
    protos.append(new WvString(proto), true);
    WvString s(proto_command("-A", proto));
    if (enable) system(s);
}


void WvIPFirewall::del_proto(WvStringParm proto)
{
    WvStringList::Iter i(protos);
    for (i.rewind(); i.next(); )
    {
	if (*i == proto)
	{
	    WvString s(proto_command("-D", proto));
	    if (enable) system(s);
	    return;
	}
    }
}


void WvIPFirewall::zap()
{
    WvIPPortAddrList::Iter i(addrs);
    for (i.rewind(); i.next(); )
    {
	del_port(*i);
	i.xunlink();
    }
    
    RedirList::Iter i2(redirs);
    for (i2.rewind(); i2.next(); )
    {
	del_redir(i2->src, i2->dstport);
	i2.xunlink();
    }
}
