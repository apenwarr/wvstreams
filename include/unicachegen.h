/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A UniConf generator that stores keys in memory.
 */
#ifndef __UNICACHEGEN_H
#define __UNICACHEGEN_H

#include "unitempgen.h"
#include "uniconftree.h"
#include "wvlog.h"

/**
 * A UniConf generator that adds a cache layer on top of another generator
 *
 * This cache implementation preloads the entire uniconf tree and then keeps up
 * to date by making changes whenever notifications are received. This means
 * that a read-only uniconfclient, when cached, will never actively contact
 * the uniconfdaemon.
 *
 * **WARNING**
 * The cache *will* go out of date if used with a uniconfclient/daemon without
 * running a select loop.
 */
class UniCacheGen : public UniTempGen
{
protected:
    WvLog log;
    IUniConfGen *inner;

    void loadtree(const UniConfKey &key = "");
    void deltacallback(const UniConfKey &key, WvStringParm value,
                       void *userdata);

public:
    UniCacheGen(IUniConfGen *_inner);
    virtual ~UniCacheGen();

    /***** Overridden members *****/
    virtual void set(const UniConfKey &key, WvStringParm value);
};

#endif // __UNICACHEGEN_H
