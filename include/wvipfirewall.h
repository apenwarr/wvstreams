/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvIPFirewall is an extremely simple hackish class that handles the Linux
 * 2.4 "iptables" firewall.  It's okay to create more than one instance
 * of this class; they'll co-operate.
 * 
 * They need you to have created the appropriate firewall tables already,
 * however, and call them from the right places in the Input and/or Forward
 * firewalls.
 */
#ifndef __WVIPFIREWALL_H
#define __WVIPFIREWALL_H

#include "wvinterface.h"
#include "wvstringlist.h"
#include "wvaddr.h"


DeclareWvList(WvIPPortAddr);

/** Class to handle Linux 2.4 IPTables */
class WvIPFirewall
{
    class Redir
    {
    public:
	WvIPPortAddr src;
	int dstport;
	
	Redir(const WvIPPortAddr &_src, int _dstport) : src(_src)
	    { dstport = _dstport; }
    };

    class RedirAll
    {
    public:
	int dstport;
	
	RedirAll(int _dstport) 
            { dstport = _dstport; }
    };

    class RedirPortRange
    {
    public:
	WvIPPortAddr src_min;
	WvIPPortAddr src_max;
	int dstport;
	
	RedirPortRange(const WvIPPortAddr &_src_min,
	    	const WvIPPortAddr &_src_max, int _dstport)
	    : src_min(_src_min), src_max(_src_max)
	    { dstport = _dstport; }
    };

    DeclareWvList(Redir);
    DeclareWvList(RedirAll);
    DeclareWvList(RedirPortRange);

    RedirList redirs;
    RedirAllList redir_alls;
    RedirPortRangeList redir_port_ranges;

    WvIPPortAddrList addrs;
    WvStringList protos;
    
    WvString port_command(const char *cmd, const char *proto,
			  const WvIPPortAddr &addr);
    WvString redir_command(const char *cmd,
			   const WvIPPortAddr &src, int dstport);
    WvString redir_port_range_command(const char *cmd,
    	const WvIPPortAddr &src_min, const WvIPPortAddr &src_max, int dstport);
    WvString redir_all_command(const char *cmd, int dstport);
    WvString proto_command(const char *cmd, const char *proto);
    WvString forward_command(const char *cmd, const char *proto,
			     const WvIPPortAddr &src,
			     const WvIPPortAddr &dst);
    WvLog log;
    const char *shutup() const
        { return ignore_errors ? " >/dev/null 2>/dev/null " : ""; }
    
public:
    WvIPFirewall();
    ~WvIPFirewall();
    
    static bool enable, ignore_errors;
    
    void zap();
    void add_port(const WvIPPortAddr &addr);
    void add_redir(const WvIPPortAddr &src, int dstport);
    void add_redir_all(int dstport);
    void add_redir_port_range(const WvIPPortAddr &src_min,
    	    const WvIPPortAddr &src_max, int dstport);
    void add_proto(WvStringParm proto);
    void add_forward(const WvIPPortAddr &src, const WvIPPortAddr &dst);
    void del_proto(WvStringParm proto);
    void del_port(const WvIPPortAddr &addr);
    void del_redir(const WvIPPortAddr &src, int dstport);
    void del_forward(const WvIPPortAddr &src, const WvIPPortAddr &dst);
    void del_redir_all(int dstport);
    void del_redir_port_range(const WvIPPortAddr &src_min,
    	    const WvIPPortAddr &src_max, int dstport);
};

#endif // __WVIPFIREWALL_H
