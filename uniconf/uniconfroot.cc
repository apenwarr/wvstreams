/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines the root management class for UniConf.  To create any kind of
 * UniConf tree, you'll need one of these.
 */
#include "uniconfroot.h"

/***** UniConfRootImpl *****/

UniConfRootImpl::UniConfRootImpl()
{
    setcallback(wvcallback(UniConfGenCallback, *this,
        UniConfRootImpl::deltacallback), NULL);
}


UniConfRootImpl::~UniConfRootImpl()
{
    // clear callback before superclasses are destroyed
    setcallback(NULL, NULL);
}


void UniConfRootImpl::deltacallback(UniConfGen *gen,
    const UniConfKey &key, void *userdata)
{
    UniWatchManager::delta(key);
}
