/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvIPFirewall is an extremely simple hackish class that handles the Linux
 * 2.1 "ipchains" firewall.  Someday, this should be rewritten and much
 * improved.
 */
#ifndef __WVIPFIREWALL_H
#define __WVIPFIREWALL_H

#include "wvlinklist.h"
#include "wvaddr.h"

DeclareWvList(WvIPPortAddr);


class WvIPFirewall
{
    WvIPPortAddrList addrs;
    
    WvString command(const char *cmd, const WvIPPortAddr &addr);
    void del(const WvIPPortAddr &addr);
    
public:
    WvIPFirewall();
    ~WvIPFirewall();
    
    void zap();
    void add(const WvIPPortAddr &addr);
};

#endif // __WVIPFIREWALL_H
