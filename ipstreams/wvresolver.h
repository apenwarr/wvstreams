/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * DNS name resolver with support for background lookups.
 */
#ifndef __WVRESOLVER_H
#define __WVRESOLVER_H

#include "wvaddr.h"
#include "wvstream.h"
#include "wvlinklist.h"

class WvResolverHostDict;
class WvResolverAddrDict;

DeclareWvList(WvIPAddr);

/**
 * ASynchronous DNS resolver functions, so that we can do non-blocking lookups
 */
class WvResolver
{
    static int numresolvers;
    static WvResolverHostDict *hostmap;
    static WvResolverAddrDict *addrmap;
public:
    WvResolver();
    ~WvResolver();
    
    /**
     * Return -1 on timeout, or the number of addresses found, which may
     * be 0 if the address does not exist.
     * addrlist, if present, has to be an initialized list.
     */
    int findaddr(int msec_timeout, const WvString &name,
		 WvIPAddr const **addr, WvIPAddrList *addrlist = NULL);
    int findname(int msec_timeout, WvIPAddr *ipaddr, char **name);
    
    /**
     * add all of our waiting fds to an fd_set for use with select().
     */
    bool select_setup(const WvString &hostname, WvStream::SelectInfo &si);
};

#endif // __WVRESOLVER_H
