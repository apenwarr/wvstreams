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
    : watchroot(NULL, UniConfKey::EMPTY)
{
    setcallback(wvcallback(UniConfGenCallback, *this,
        UniConfRootImpl::gen_callback), NULL);
}


UniConfRootImpl::~UniConfRootImpl()
{
    // clear callback before superclasses are destroyed
    setcallback(NULL, NULL);
}


void UniConfRootImpl::add_callback(const UniConfKey &key,
    const UniConfCallback &callback, void *userdata, bool recurse)
{
    UniWatch *w = new UniWatch(recurse, callback, userdata);

    UniWatchTree *node = & watchroot;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); )
    {
        UniWatchTree *prev = node;
        node = node->findchild(it());
        if (! node)
            node = new UniWatchTree(prev, it());
    }
    node->watches.append(w, true);
}


void UniConfRootImpl::del_callback(const UniConfKey &key,
    const UniConfCallback &callback, void *userdata, bool recurse)
{
    UniWatch needle(recurse, callback, userdata);
    UniWatchTree *node = watchroot.find(key);
    if (node)
    {
        UniWatchList::Iter it(node->watches);
        for (it.rewind(); it.next(); )
        {
            UniWatch *w = it.ptr();
            if (needle == *w)
            {
                // remove the watch
                it.xunlink();
            }
        }
        // prune the branch if needed
        prune(node);
    }
}


void UniConfRootImpl::add_setbool(const UniConfKey &key, bool *flag,
                                  bool recurse)
{
    add_callback(key, wvcallback(UniConfCallback, *this,
        UniConfRootImpl::setbool_callback), flag, recurse);
}


void UniConfRootImpl::del_setbool(const UniConfKey &key, bool *flag,
                                  bool recurse)
{
    del_callback(key, wvcallback(UniConfCallback, *this,
        UniConfRootImpl::setbool_callback), flag, recurse);
}


void UniConfRootImpl::check(UniWatchTree *node,
    const UniConfKey &key, int segleft)
{
    UniWatchList::Iter it(node->watches);
    for (it.rewind(); it.next(); )
    {
        UniWatch *w = it.ptr();
        if (!w->recursive() && segleft > 0)
            continue;

        w->notify(UniConf(this, key));
    }
}


void UniConfRootImpl::deletioncheck(UniWatchTree *node, const UniConfKey &key)
{
    UniWatchTree::Iter it(*node);
    for (it.rewind(); it.next(); )
    {
        UniWatchTree *w = it.ptr();
        UniConfKey subkey(key, w->key());
        
        // pretend that we wiped out just this key
        check(w, subkey, 0);
        deletioncheck(w, subkey);
    }
}


void UniConfRootImpl::prune(UniWatchTree *node)
{
    while (node != & watchroot && ! node->isessential())
    {
        UniWatchTree *next = node->parent();
        delete node;
        node = next;
    }
}


void UniConfRootImpl::setbool_callback(const UniConf &cfg, void *userdata)
{
    bool *flag = static_cast<bool*>(userdata);
    *flag = true;
}


void UniConfRootImpl::gen_callback(const UniConfKey &key, WvStringParm value,
                                   void *userdata)
{
    hold_delta();
    
    UniWatchTree *node = & watchroot;
    int segs = key.numsegments();

    // check root node
    check(node, key, segs);
    
    // look for watches on key and its ancestors
    for (int s = 0; s < segs;)
    {
        node = node->findchild(key.segment(s));
        s++;
        if (! node)
            goto done; // no descendents so we can stop
        check(node, key, segs - s);
    }

    // look for watches on descendents of key if node was deleted
    if (value.isnull())
        deletioncheck(node, key);
    else
        check(node, key, 0);
    
done:
    unhold_delta();
}
