/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines the root management class for UniConf.
 */
#ifndef __UNICONFROOT_H
#define __UNICONFROOT_H

#include "uniconfmounttree.h"
#include "uniconfwatch.h"

/** The UniConfRoot implementation. */
class UniConfRootImpl : public UniConfMountTreeGen
{
    /** undefined. */
    UniConfRootImpl(const UniConfRootImpl &other);

public:
    /** Creates an empty UniConf tree with no mounted stores. */
    UniConfRootImpl();

    /** Destroys the UniConf tree along with all uncommitted data. */
    ~UniConfRootImpl();
    
    /**
     * Requests notification when any the keys covered by the
     * recursive depth specification changes.
     * "depth" is the recursion depth identifying keys of interest
     * "watch" is the observer to notify
     */
    void addwatch(const UniConfKey &key, UniConfDepth::Type depth,
        UniConfWatch *watch);

    /**
     * Cancels a previously registered notification request.
     * "depth" is the recursion depth identifying keys of interest
     * "watch" is the observer to notify
     */
    void delwatch(const UniConfKey &key, UniConfDepth::Type depth,
        UniConfWatch *watch);
        
private:
    void deltacallback(const UniConfGen &gen, const UniConfKey &key,
        UniConfDepth::Type depth, void *userdata);
};

#endif //__UNICONFROOT_H
