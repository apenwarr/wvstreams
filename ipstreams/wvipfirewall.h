/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvIPFirewall is an extremely simple hackish class that handles the Linux
 * 2.1 "ipchains" firewall.  Someday, this might be rewritten and much
 * improved.
 */
#ifndef __WVIPFIREWALL_H
#define __WVIPFIREWALL_H

#include "wvlinklist.h"
#include "wvaddr.h"



DeclareWvList(WvIPPortAddr);


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

    DeclareWvList(Redir);

    RedirList redirs;
    WvIPPortAddrList addrs;
    
    WvString port_command(const char *cmd, const char *proto,
			  const WvIPPortAddr &addr);
    WvString redir_command(const char *cmd,
			   const WvIPPortAddr &src, int dstport);
    
    void del_port(const WvIPPortAddr &addr);
    void del_redir(const WvIPPortAddr &src, int dstport);
    
public:
    WvIPFirewall();
    ~WvIPFirewall();
    
    static bool enable;
    
    void zap();
    void add_port(const WvIPPortAddr &addr);
    void add_redir(const WvIPPortAddr &src, int dstport);
};

#endif // __WVIPFIREWALL_H
