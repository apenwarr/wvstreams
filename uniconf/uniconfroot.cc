/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines the root management class for UniConf.  To create any kind of
 * UniConf tree, you'll need one of these.
 */
#include "uniconfroot.h"
#include "uniconfgen.h"
#include "uniconf.h"
#include "wvmoniker.h"


/***** UniConfRoot *****/

UniConfRoot::UniConfRoot()
    : UniConf(this, UniConfKey::EMPTY)
{
    mounts = new UniConfMountTree(NULL, UniConfKey::EMPTY);
}


UniConfRoot::UniConfRoot(WvStringParm moniker, bool refresh)
    : UniConf(this, UniConfKey::EMPTY)
{
    mounts = new UniConfMountTree(NULL, UniConfKey::EMPTY);
    _mount(UniConfKey::EMPTY, moniker, refresh);
}


UniConfRoot::UniConfRoot(UniConfGen *gen, bool refresh)
    : UniConf(this, UniConfKey::EMPTY)
{
    mounts = new UniConfMountTree(NULL, UniConfKey::EMPTY);
    _mountgen(UniConfKey::EMPTY, gen, refresh);
}


UniConfRoot::~UniConfRoot()
{
    // destroys all generators
    delete mounts;
}

WvString UniConfRoot::_get(const UniConfKey &key)
{
    // consult the generators
    UniConfMountTree::GenIter it(*mounts, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        WvString result = gen->get(it.tail());
        if (!result.isnull())
            return result;
    }
    
    // ensure key exists if it is in the path of a mountpoint
    UniConfMountTree *node = mounts->find(key);
    if (node)
        return ""; // fake read-only key not provided by anyone

    // no matches
    return WvString::null;
}


bool UniConfRoot::_set(const UniConfKey &key, WvStringParm value)
{
    // update the generator that defines the key, if any
    UniConfKey mountpoint;
    UniConfGen *provider = _whichmount(key, &mountpoint);
    if (provider)
        return provider->set(mountpoint, value);
    return false;
}


bool UniConfRoot::_zap(const UniConfKey &key)
{
    bool success = true;
    UniConfMountTree::GenIter it(*mounts, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (!gen->zap(it.tail()))
            success = false;
    }
    // FIXME: need to recurse over generators of descendent keys
    //        to zap all of their contents also
    return success;
}


bool UniConfRoot::_exists(const UniConfKey &key)
{
    // ensure key exists if it is in the path of a mountpoint
    UniConfMountTree *node = mounts->find(key);
    if (node)
        return true;
    
    // otherwise consult the generators
    UniConfMountTree::GenIter it(*mounts, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->exists(it.tail()))
            return true;
    }

    // no match
    return false;
}


bool UniConfRoot::_haschildren(const UniConfKey &key)
{
    UniConfMountTree *node = mounts->find(key);
    if (node && node->haschildren())
        return true;

    UniConfMountTree::GenIter it(*mounts, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->haschildren(it.tail()))
            return true;
    }
    return false;
}


bool UniConfRoot::_refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    return dorecursive(genrefreshfunc, key, depth);
}


bool UniConfRoot::genrefreshfunc(UniConfGen *gen,
    const UniConfKey &key, UniConfDepth::Type depth)
{
    return gen->refresh(key, depth);
}


bool UniConfRoot::_commit(const UniConfKey &key, UniConfDepth::Type depth)
{
    return dorecursive(gencommitfunc, key, depth);
}


bool UniConfRoot::gencommitfunc(UniConfGen *gen,
		const UniConfKey &key, UniConfDepth::Type depth)
{
    return gen->commit(key, depth);
}


bool UniConfRoot::dorecursive(GenFunc func, const UniConfKey &key,
			      UniConfDepth::Type depth)
{
    // do containing generators
    bool success = true;
    UniConfMountTree::GenIter it(*mounts, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (!func(gen, it.tail(), depth))
            success = false;
    }

    // do recursive
    if (depth != UniConfDepth::ZERO)
    {
        UniConfMountTree *node = mounts->find(key);
        if (node && ! dorecursivehelper(func, node, depth))
            success = false;
    }
    return success;
}


bool UniConfRoot::dorecursivehelper(GenFunc func,
    UniConfMountTree *node, UniConfDepth::Type depth)
{
    // determine depth for next step
    switch (depth)
    {
        case UniConfDepth::ZERO:
            assert(false);
            return true;

        case UniConfDepth::ONE:
        case UniConfDepth::CHILDREN:
            depth = UniConfDepth::ZERO;
            break;

        case UniConfDepth::DESCENDENTS:
            depth = UniConfDepth::INFINITE;
        case UniConfDepth::INFINITE:
            break;
    }

    // process nodes and recurse if needed
    bool success = true;
    UniConfMountTree::Iter it(*node);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it->generator;
        if (gen)
        {
            if (! func(gen, UniConfKey::EMPTY, depth))
                success = false;
        }
        if (depth != UniConfDepth::ZERO)
        {
            if (! dorecursivehelper(func, it.ptr(), depth))
                success = false;
        }
    }
    return success;
}
 
 
void UniConfRoot::_addwatch(const UniConfKey &key,
    UniConfDepth::Type depth, UniConfWatch *watch)
{
}


void UniConfRoot::_delwatch(const UniConfKey &key,
    UniConfDepth::Type depth, UniConfWatch *watch)
{
}


void UniConfRoot::delta(const UniConfKey &key, UniConfDepth::Type depth)
{
}


UniConfGen *UniConfRoot::_mount(const UniConfKey &key,
    WvStringParm moniker, bool refresh)
{
    UniConfGen *gen = wvcreate<UniConfGen>(moniker);
    if (gen)
        _mountgen(key, gen, refresh); // assume always succeeds for now
    return gen;
}


UniConfGen *UniConfRoot::_mountgen(const UniConfKey &key,
    UniConfGen *gen, bool refresh)
{
    UniConfMountTree *node = mounts->findormake(key);
    node->generator = gen;
    gen->setcallback(wvcallback(UniConfGenCallback, *this,
        UniConfRoot::gencallback), node);
    if (gen && refresh)
        gen->refresh(UniConfKey::EMPTY, UniConfDepth::INFINITE);
    return gen;
}


void UniConfRoot::_unmount(const UniConfKey &key, bool commit)
{
    UniConfMountTree *node = mounts->find(key);
    if (!node)
        return;
    UniConfGen *gen = node->generator;
    if (!gen)
        return;
    if (commit)
        gen->commit(UniConfKey::EMPTY, UniConfDepth::INFINITE);
    node->generator = NULL;
    gen->setcallback(NULL, NULL);
    delete gen;
}


UniConfGen *UniConfRoot::_whichmount(const UniConfKey &key,
				    UniConfKey *mountpoint)
{
    // see if a generator acknowledges the key
    UniConfMountTree::GenIter it(*mounts, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->exists(it.tail()))
            goto found;
    }
    // find the generator that would be used to set the value
    it.rewind();
    if (! it.next())
        return NULL;

found:
    if (mountpoint)
        *mountpoint = it.tail();
    return it.ptr();
}


void UniConfRoot::prune(UniConfMountTree *node)
{
    while (node != mounts && !node->isessential())
    {
        UniConfMountTree *next = node->parent();
        delete node;
        node = next;
    }
}


void UniConfRoot::gencallback(const UniConfGen &gen,
    const UniConfKey &key, UniConfDepth::Type depth, void *userdata)
{
    UniConfMountTree *node = static_cast<UniConfMountTree*>(userdata);
    delta(UniConfKey(node->fullkey(), key), depth);
}



/***** UniConfRoot::BasicIter *****/

UniConfRoot::BasicIter::BasicIter(UniConfRoot &root, const UniConfKey &key) 
    : xroot(&root), xkey(key), genit(*root.mounts, key),
	hack(71), hackit(hack)
{
}


void UniConfRoot::BasicIter::rewind()
{
    hack.zap();

    // add mountpoint nodes
    UniConfMountTree *node = xroot->mounts->find(xkey);
    if (node)
    {
        UniConfMountTree::Iter nodeit(*node);
        for (nodeit.rewind(); nodeit.next(); )
        {
            hack.add(new WvString(nodeit->key()), true);
        }
    }

    // add key nodes
    for (genit.rewind(); genit.next(); )
    {
        UniConfGen *gen = genit.ptr();
        UniConfAbstractIter *keyit = gen->iterator(genit.tail());
        for (keyit->rewind(); keyit->next(); )
        {
            hack.add(new WvString(keyit->key()), true);
        }
        delete keyit;
    }

    hackit.rewind();
}


bool UniConfRoot::BasicIter::next()
{
    return hackit.next();
}


UniConfKey UniConfRoot::BasicIter::key() const
{
    return UniConfKey(hackit());
}



/***** UniConfMountTree *****/

UniConfMountTree::UniConfMountTree(UniConfMountTree *parent,
    const UniConfKey &key) :
    UniConfTree<UniConfMountTree>(parent, key), generator(NULL)
{
}


UniConfMountTree::~UniConfMountTree()
{
    delete generator;
}


UniConfMountTree *UniConfMountTree::findnearest(const UniConfKey &key,
    int &split)
{
    split = 0;
    UniConfMountTree *node = this;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); ++split)
    {
        UniConfMountTree *next = node->findchild(it());
        if (! next)
            break;
        node = next;
    }
    return node;
}


UniConfMountTree *UniConfMountTree::findormake(const UniConfKey &key)
{
    UniConfMountTree *node = this;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); )
    {
        UniConfMountTree *prev = node;
        node = prev->findchild(it());
        if (! node)
            node = new UniConfMountTree(prev, it());
    }
    return node;
}



/***** UniConfMountTree::MountIter *****/

UniConfMountTree::MountIter::MountIter(UniConfMountTree &root,
    const UniConfKey &key) :
    xkey(key)
{
    bestnode = root.findnearest(key, bestsplit);
}


void UniConfMountTree::MountIter::rewind()
{
    xnode = NULL;
}


bool UniConfMountTree::MountIter::next()
{
    if (! xnode)
    {
        xsplit = bestsplit;
        xnode = bestnode;
    }
    else if (xsplit != 0)
    {
        xsplit -= 1;
        xnode = xnode->parent();
    }
    else
        return false;
    return true;
}



/***** UniConfMountTree::GenIter *****/

UniConfMountTree::GenIter::GenIter(UniConfMountTree &root,
    const UniConfKey &key) :
    UniConfMountTree::MountIter(root, key)
{
}


void UniConfMountTree::GenIter::rewind()
{
    UniConfMountTree::MountIter::rewind();
}


bool UniConfMountTree::GenIter::next()
{
    // find the next node up that actually has a generator
    while (UniConfMountTree::MountIter::next())
    {
        if (node()->generator)
            return true;
    }
    return false;
}
