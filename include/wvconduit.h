
/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 */ 
#ifndef __WVCONDUIT_H
#define __WVCONDUIT_H

#include "wvfdstream.h"

/**
 * Implementation of the WvConduit stream.  WvConduit uses a
 * socketpair() to create a pair of streams linked in a way that
 * allows you to read from one whatever is written to the other.
 */
class WvConduit : public WvFDStream
{
public:
    WvConduit();
    WvConduit *get_slave();
    void shutdown();
    virtual ~WvConduit();
protected:
    WvConduit(WvConduit* master, int fd);
    WvConduit* other;
    bool slave;
};

#endif // __WVCONDUIT_H
