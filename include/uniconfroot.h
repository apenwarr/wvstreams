/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines the root management class for UniConf.
 */
#ifndef __UNICONFROOT_H
#define __UNICONFROOT_H

#include "unimounttreegen.h"
#include "uniconfwatch.h"

/**
 * The UniConfRoot implementation.
 *
 * Wires together all of the bits and pieces that make up the core structure
 * of a UniConf tree.  These pieces are factored to make them easier to
 * understand as independent units.  They may also be used independently
 * to achieve a variety of interesting effects.
 * 
 * For tree contents and mounting support see UniMountTreeGen.
 * For notification support see UniWatchManager.
 */
class UniConfRootImpl : public UniMountTreeGen, public UniWatchManager
{
    /** undefined. */
    UniConfRootImpl(const UniConfRootImpl &other);

public:
    /** Creates an empty UniConf tree with no mounted stores. */
    UniConfRootImpl();

    /** Destroys the UniConf tree along with all uncommitted data. */
    ~UniConfRootImpl();

private:
    void deltacallback(UniConfGen *gen, const UniConfKey &key,
        void *userdata);
};

#endif //__UNICONFROOT_H
