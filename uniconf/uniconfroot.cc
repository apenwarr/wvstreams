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
    const UniConfCallback &callback, void *userdata,
    UniConfDepth::Type depth)
{
    UniWatch *w = new UniWatch(depth, callback, userdata);

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
    const UniConfCallback &callback, void *userdata,
    UniConfDepth::Type depth)
{
    UniWatch needle(depth, callback, userdata);
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
    UniConfDepth::Type depth)
{
    add_callback(key, wvcallback(UniConfCallback, *this,
        UniConfRootImpl::setbool_callback), flag, depth);
}


void UniConfRootImpl::del_setbool(const UniConfKey &key, bool *flag,
    UniConfDepth::Type depth)
{
    del_callback(key, wvcallback(UniConfCallback, *this,
        UniConfRootImpl::setbool_callback), flag, depth);
}


void UniConfRootImpl::check(UniWatchTree *node,
    const UniConfKey &key, int segleft)
{
    UniWatchList::Iter it(node->watches);
    for (it.rewind(); it.next(); )
    {
        UniWatch *w = it.ptr();
        switch (w->depth())
        {
            case UniConfDepth::ZERO:
                if (segleft > 0)
                    continue;
                break;
                
            case UniConfDepth::ONE:
                if (segleft > 1)
                    continue;
                break;

            case UniConfDepth::INFINITE:
                // always matches
                break;

            case UniConfDepth::CHILDREN:
                if (segleft > 1)
                    continue;
                break;

            case UniConfDepth::DESCENDENTS:
                // always matches
                break;
        }
        w->notify(UniConf(this, key));
    }
}


void UniConfRootImpl::recursivecheck(UniWatchTree *node,
    const UniConfKey &key, int segleft)
{
    UniWatchTree::Iter it(*node);
    segleft -= 1;
    for (it.rewind(); it.next(); )
    {
        UniWatchTree *w = it.ptr();
        UniConfKey subkey(key, w->key());
        
        // pretend that we wiped out just this key
        check(w, subkey, segleft);
        recursivecheck(w, subkey, segleft);
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


void UniConfRootImpl::gen_callback(UniConfGen *gen,
    const UniConfKey &key, void *userdata)
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
        s += 1;
        if (! node)
            goto done; // no descendents so we can stop
        check(node, key, segs - s);
    }

    // look for watches on descendents of key
    recursivecheck(node, key, 0);
    
done:
    unhold_delta();
}
