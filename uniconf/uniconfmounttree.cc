/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a UniConfGen that manages a tree of UniConfGen instances.
 */
#include "uniconfmounttree.h"
#include "wvmoniker.h"

/***** UniConfMountTreeGen *****/

UniConfMountTreeGen::UniConfMountTreeGen()
{
    mounts = new UniConfMountTree(NULL, UniConfKey::EMPTY);
}


UniConfMountTreeGen::~UniConfMountTreeGen()
{
    // destroys all generators
    delete mounts;
}


WvString UniConfMountTreeGen::get(const UniConfKey &key)
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


bool UniConfMountTreeGen::set(const UniConfKey &key, WvStringParm value)
{
    // update the generator that defines the key, if any
    UniConfKey mountpoint;
    UniConfGen *provider = whichmount(key, &mountpoint);
    if (provider)
        return provider->set(mountpoint, value);
    return false;
}


bool UniConfMountTreeGen::zap(const UniConfKey &key)
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


bool UniConfMountTreeGen::exists(const UniConfKey &key)
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


bool UniConfMountTreeGen::haschildren(const UniConfKey &key)
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


bool UniConfMountTreeGen::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    return dorecursive(genrefreshfunc, key, depth);
}


bool UniConfMountTreeGen::genrefreshfunc(UniConfGen *gen,
    const UniConfKey &key, UniConfDepth::Type depth)
{
    return gen->refresh(key, depth);
}


bool UniConfMountTreeGen::commit(const UniConfKey &key, UniConfDepth::Type depth)
{
    return dorecursive(gencommitfunc, key, depth);
}


bool UniConfMountTreeGen::gencommitfunc(UniConfGen *gen,
    const UniConfKey &key, UniConfDepth::Type depth)
{
    return gen->commit(key, depth);
}


bool UniConfMountTreeGen::dorecursive(GenFunc func, const UniConfKey &key,
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


bool UniConfMountTreeGen::dorecursivehelper(GenFunc func,
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
        UniConfGenList::Iter genit(it->generators);
        for (genit.rewind(); genit.next(); )
        {
            if (! func(genit.ptr(), UniConfKey::EMPTY, depth))
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
 
 
UniConfGen *UniConfMountTreeGen::mount(const UniConfKey &key,
    WvStringParm moniker, bool refresh)
{
    UniConfGen *gen = wvcreate<UniConfGen>(moniker);
    if (gen)
        mountgen(key, gen, refresh); // assume always succeeds for now
    return gen;
}


UniConfGen *UniConfMountTreeGen::mountgen(const UniConfKey &key,
    UniConfGen *gen, bool refresh)
{
    UniConfMountTree *node = mounts->findormake(key);
    node->generators.append(gen, true);
    gen->setcallback(wvcallback(UniConfGenCallback, *this,
        UniConfMountTreeGen::gencallback), node);
    if (gen && refresh)
        gen->refresh(UniConfKey::EMPTY, UniConfDepth::INFINITE);
    return gen;
}


void UniConfMountTreeGen::unmount(const UniConfKey &key,
    UniConfGen *gen, bool commit)
{
    UniConfMountTree *node = mounts->find(key);
    if (!node)
        return;

    UniConfGenList::Iter genit(node->generators);
    if (! genit.find(gen))
        return;

    if (commit)
        gen->commit(UniConfKey::EMPTY, UniConfDepth::INFINITE);
    gen->setcallback(NULL, NULL);

    node->generators.unlink(gen);
}


UniConfGen *UniConfMountTreeGen::whichmount(const UniConfKey &key,
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


bool UniConfMountTreeGen::ismountpoint(const UniConfKey &key)
{
    UniConfMountTree *node = mounts->find(key);
    return node && ! node->generators.isempty();
}


UniConfMountTreeGen::Iter *UniConfMountTreeGen::iterator(const UniConfKey &key)
{
    return new Iter(*this, key);
}


void UniConfMountTreeGen::prune(UniConfMountTree *node)
{
    while (node != mounts && !node->isessential())
    {
        UniConfMountTree *next = node->parent();
        delete node;
        node = next;
    }
}


void UniConfMountTreeGen::gencallback(const UniConfGen &gen,
    const UniConfKey &key, UniConfDepth::Type depth, void *userdata)
{
    UniConfMountTree *node = static_cast<UniConfMountTree*>(userdata);
    delta(UniConfKey(node->fullkey(), key), depth);
}


/***** UniConfMountTreeGen::Iter *****/

UniConfMountTreeGen::Iter::Iter(UniConfMountTreeGen &root, const UniConfKey &key) 
    : xroot(&root), xkey(key), genit(*root.mounts, key),
        hack(71), hackit(hack)
{
}


void UniConfMountTreeGen::Iter::rewind()
{
    hack.zap();

    // find nodes provided by the root of any mount points.
    // (if we want to iterate over /foo, then mounts directly on /foo/blah and
    // /foo/snoo must be included)
    UniConfMountTree *node = xroot->mounts->find(xkey);
    if (node)
    {
        UniConfMountTree::Iter nodeit(*node);
        for (nodeit.rewind(); nodeit.next(); )
            hack.add(new WvString(nodeit->key()), true);
    }

    // walk through *all* generators and add any appropriate sub-keys
    // provided by each generator.
    for (genit.rewind(); genit.next(); )
    {
        UniConfGen *gen = genit.ptr();
        UniConfAbstractIter *keyit = gen->iterator(genit.tail());
        for (keyit->rewind(); keyit->next(); )
            hack.add(new WvString(keyit->key()), true);
        delete keyit;
    }

    hackit.rewind();
}


bool UniConfMountTreeGen::Iter::next()
{
    return hackit.next();
}


UniConfKey UniConfMountTreeGen::Iter::key() const
{
    return UniConfKey(hackit());
}



/***** UniConfMountTree *****/

UniConfMountTree::UniConfMountTree(UniConfMountTree *parent,
    const UniConfKey &key) :
    UniConfTree<UniConfMountTree>(parent, key)
{
}


UniConfMountTree::~UniConfMountTree()
{
}


UniConfMountTree *UniConfMountTree::findnearest(const UniConfKey &key,
    int &split)
{
    split = 0;
    UniConfMountTree *node = this;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); split++)
    {
        UniConfMountTree *next = node->findchild(it());
        if (!next)
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
        if (!node)
            node = new UniConfMountTree(prev, it());
    }
    return node;
}



/***** UniConfMountTree::MountIter *****/

UniConfMountTree::MountIter::MountIter(UniConfMountTree &root,
    const UniConfKey &key)
    : xkey(key)
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
    UniConfMountTree::MountIter(root, key),
    genit(NULL)
{
}


UniConfMountTree::GenIter::~GenIter()
{
    delete genit;
}


void UniConfMountTree::GenIter::rewind()
{
    if (genit)
    {
        delete genit;
        genit = NULL;
    }
    UniConfMountTree::MountIter::rewind();
}


bool UniConfMountTree::GenIter::next()
{
    for (;;)
    {
        if (genit && genit->next())
            return true;
        if (! UniConfMountTree::MountIter::next())
            return false;

        genit = new UniConfGenList::Iter(node()->generators);
        genit->rewind();
    }
    return false;
}
