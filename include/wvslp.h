/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * OpenSLP Service Lister
 */

#ifndef WVSLP_H
#define WVSLP_H

#include "wvautoconf.h"

#include "wvstringlist.h"
#include "wvlog.h"
#include "wverror.h"

typedef void* SLPHandle;

/**
 * Get a list of servers that provide the requested service
 * returns false only if the SLP service failed... a list
 * with 0 values is perfectly legal.
 */
bool slp_get_servs(WvStringParm service, WvStringList &list);

/**
 * Advertise yourself as an SLP Service
 */
class WvSlp 
{
public:
    /**
     * Start up the necessary SLP Service bits
     */
    WvSlp();
    
    /**
     * Shutdown and deregister all SLP Services
     */
    ~WvSlp();
    
    /**
     * Start advertising an SLP Service
     */
    void add_service(WvStringParm servicename, WvStringParm hostname, 
		     WvStringParm port);

private:
    SLPHandle hslp;

    WvLog log;
    WvError err;
    WvStringList services;
};

#endif /* WVSLP_H */
