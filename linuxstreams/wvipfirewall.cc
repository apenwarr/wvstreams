/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvIPFirewall is an extremely simple hackish class that handles the Linux
 * 2.4 "iptables" firewall.  See wvipfirewall.h.
 */
#include "wvipfirewall.h"
#include "wvinterface.h"
#include <unistd.h>


bool WvIPFirewall::enable = false, WvIPFirewall::ignore_errors = true;


WvIPFirewall::WvIPFirewall() : log("Firewall", WvLog::Debug2)
{
    // don't change any firewall rules here!  Remember that there may be
    // more than one instance of the firewall object.
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

WvString WvIPFirewall::forward_command(const char *cmd, 
				       const char *proto,
				       const WvIPPortAddr &src,
				       const WvIPPortAddr &dst)
{
    WvIPAddr srcaddr(src), dstaddr(dst), zero;
    WvString haveiface(""), haveoface("");
    if (!(srcaddr == zero))
    {
	haveiface.append("-d ");
	haveiface.append((WvString)srcaddr);
    }

    if (!(dstaddr == zero))
    {
	haveoface.append("-d ");
	haveoface.append((WvString)dstaddr);
    }

    WvString retval("iptables -t nat %s OFASTFORWARD -p %s -m mark --mark 0xBEEF "
                    "--dport %s %s -j MASQUERADE %s \n", 
                    cmd, proto, dst.port, haveoface, shutup());

    retval.append("iptables -t nat %s FASTFORWARD -p %s --dport %s %s "
                  "-j DNAT --to-destination %s "
                  "%s \n", cmd, proto, src.port, haveiface,  dst, shutup());
    
    retval.append("iptables %s FFASTFORWARD -j ACCEPT -p %s "
		  "--dport %s %s \n "
		  "%s\n", cmd, proto, src.port,
		  haveiface, shutup());

    retval.append("iptables %s FFASTFORWARD -j ACCEPT -p %s "
		  "--dport %s %s "
		  "%s\n", cmd, proto, dst.port,
		  haveoface, shutup());
    
    return retval;
}

WvString WvIPFirewall::redir_all_command(const char *cmd, int dstport)
{
    return WvString("iptables -t nat %s TProxy "
		    "-p tcp "
		    "-j REDIRECT --to-ports %s "
		    "%s",
		    cmd,
		    dstport,
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

void WvIPFirewall::add_forward(const WvIPPortAddr &src,
			       const WvIPPortAddr &dst)
{
    forwards.append(new FastForward(src, dst), true);
    WvString s(forward_command("-A", "tcp", src, dst)),
    	    s2(forward_command("-A", "udp", src, dst));

    log("Add Forwards (%s):\n%s\n%s\n", enable, s, s2);
    
    if (enable)
    {
	system(s);
	system(s2);
    }
}

void WvIPFirewall::del_forward(const WvIPPortAddr &src)
{
     FastForwardList::Iter i(forwards);
     log("Find this Forward %s\n", (WvString)src);
     for (i.rewind(); i.next(); )
     {
	 log("Find Forward %s, %s\n", (WvString)i->src, (WvString)i->dst);
	 if (i->src == src)
	 {
	     WvString s(forward_command("-D", "tcp", src, i->dst)),
	     s2(forward_command("-D", "udp", src, i->dst));
	     log("Delete Forward (%s):\n%s\n%s\n", enable, s, s2);
	     if (enable) 
	     {
		 system(s);
		 system(s2);
	     }
	     i.unlink();
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

void WvIPFirewall::add_redir_all(int dstport)
{
    redir_alls.append(new RedirAll(dstport), true);
    WvString s(redir_all_command("-A", dstport));
    if (enable) system(s);
}


void WvIPFirewall::del_redir_all(int dstport)
{
    RedirAllList::Iter i(redir_alls);
    for (i.rewind(); i.next(); )
    {
	if (i->dstport == dstport)
	{
	    WvString s(redir_all_command("-D", dstport));
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


// clear out our portion of the firewall
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
    
    RedirAllList::Iter i2_5(redir_alls);
    for (i2_5.rewind(); i2_5.next(); )
    {
	del_redir_all(i2_5->dstport);
	i2_5.xunlink();
    }
    
    WvStringList::Iter i3(protos);
    for (i3.rewind(); i3.next(); )
    {
        del_proto(*i3);
        i3.xunlink();
    }
}
