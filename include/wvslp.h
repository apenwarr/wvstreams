/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * OpenSLP Service Lister
 */

#ifndef WVSLP_H
#define WVSLP_H

#include "wvautoconf.h"

    
#ifdef WITH_SLP
#include "wvstring.h"
class WvStringList;

/**
 * Get a list of servers that provide the requested service
 * returns false only if the SLP service failed... a list
 * with 0 values is perfectly legal.
 */
bool slp_get_servs(WvStringParm service, WvStringList &list);

#endif /* WITH_SLP */
#endif /* WVSLP_H */
