/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Defines the root management class for UniConf.
 */
#include "uniconfroot.h"
#include "uniconfgen.h"
#include "wvstreamlist.h"


/***** UniConfRoot *****/

UniConfRoot::UniConfRoot() :
    root(NULL), streamlist(NULL)
{
    root = new UniConfInfoTree(NULL, UniConfKey::EMPTY);
}


UniConfRoot::~UniConfRoot()
{
    if (streamlist)
        detach(streamlist);
    // destroys all generators
    delete root;
}


WvString UniConfRoot::get(const UniConfKey &key)
{
    UniConfInfoTree::GenIter it(*root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        WvString result = gen->get(it.tail());
        if (! result.isnull())
            return result;
    }
    return WvString::null;
}


bool UniConfRoot::set(const UniConfKey &key, WvStringParm value)
{
    // update the generator that defines the key, if any
    UniConfKey mountpoint;
    UniConfGen *provider = whichmount(key, & mountpoint);
    if (provider)
        return provider->set(mountpoint, value);
    return false;
}


bool UniConfRoot::zap(const UniConfKey &key)
{
    bool success = true;
    UniConfInfoTree::GenIter it(*root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (! gen->zap(it.tail()))
            success = false;
    }
    return success;
}


bool UniConfRoot::exists(const UniConfKey &key)
{
    UniConfInfoTree *node = root->find(key);
    if (node)
        return true;
        
    UniConfInfoTree::GenIter it(*root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->exists(it.tail()))
            return true;
    }
    return false;
}


bool UniConfRoot::haschildren(const UniConfKey &key)
{
    UniConfInfoTree *node = root->find(key);
    if (node && node->haschildren())
        return true;

    UniConfInfoTree::GenIter it(*root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->haschildren(it.tail()))
            return true;
    }
    return false;
}


bool UniConfRoot::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    return dorecursive(genrefreshfunc, key, depth);
}


bool UniConfRoot::genrefreshfunc(UniConfGen *gen,
    const UniConfKey &key, UniConfDepth::Type depth)
{
    return gen->refresh(key, depth);
}


bool UniConfRoot::commit(const UniConfKey &key, UniConfDepth::Type depth)
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
    UniConfInfoTree::GenIter it(*root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (! func(gen, it.tail(), depth))
            success = false;
    }

    // do recursive
    if (depth != UniConfDepth::ZERO)
    {
        UniConfInfoTree *node = root->find(key);
        if (node && ! dorecursivehelper(func, node, depth))
            success = false;
    }
    return success;
}


bool UniConfRoot::dorecursivehelper(GenFunc func,
    UniConfInfoTree *node, UniConfDepth::Type depth)
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
    UniConfInfoTree::Iter it(*node);
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
 
 
void UniConfRoot::attach(WvStreamList *_streamlist)
{
    assert(streamlist == NULL);
    assert(_streamlist != NULL);
    streamlist = _streamlist;
    recursiveattach(root);
}


void UniConfRoot::recursiveattach(UniConfInfoTree *node)
{
    UniConfGen *gen = node->generator;
    if (gen)
        gen->attach(streamlist);
    UniConfInfoTree::Iter it(*node);
    for (it.rewind(); it.next(); )
        recursiveattach(it.ptr());
}


void UniConfRoot::detach(WvStreamList *_streamlist)
{
    assert(streamlist != NULL);
    assert(streamlist == _streamlist);
    recursivedetach(root);
    streamlist = NULL;
}


void UniConfRoot::recursivedetach(UniConfInfoTree *node)
{
    UniConfGen *gen = node->generator;
    if (gen)
        gen->detach(streamlist);
    UniConfInfoTree::Iter it(*node);
    for (it.rewind(); it.next(); )
        recursivedetach(it.ptr());
}


UniConfGen *UniConfRoot::mount(const UniConfKey &key,
    const UniConfLocation &location)
{
    UniConfGen *gen = UniConfGenFactoryRegistry::instance()->
        newgen(location);
    mountgen(key, gen); // assume always succeeds for now
    return gen;
}


UniConfGen *UniConfRoot::mountgen(const UniConfKey &key,
    UniConfGen *gen)
{
    UniConfInfoTree *node = root->findormake(key);
    node->generator = gen;
    if (streamlist)
        gen->attach(streamlist);
    return gen;
}


void UniConfRoot::unmount(const UniConfKey &key, UniConfGen *xgen)
{
    UniConfInfoTree *node = root->find(key);
    if (! node)
        return;
    UniConfGen *gen = node->generator;
    if (! gen)
        return;

    node->generator = NULL;
    if (streamlist)
        gen->detach(streamlist);
    delete gen;
}


UniConfGen *UniConfRoot::whichmount(const UniConfKey &key,
    UniConfKey *mountpoint)
{
    // see if a generator acknowledges the key
    UniConfInfoTree::GenIter it(*root, key);
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


void UniConfRoot::prune(UniConfInfoTree *node)
{
    while (node != root && ! node->isessential())
    {
        UniConfInfoTree *next = node->parent();
        delete node;
        node = next;
    }
}



/***** UniConfRoot::Iter *****/

UniConfRoot::Iter::Iter(UniConfRoot &root, const UniConfKey &key) :
    xroot(& root), xkey(key), genit(*root.root, key),
    hack(71), hackit(hack)
{
}


void UniConfRoot::Iter::rewind()
{
    hack.zap();

    // add mountpoint nodes
    UniConfInfoTree *node = xroot->root->find(xkey);
    if (node)
    {
        UniConfInfoTree::Iter nodeit(*node);
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


bool UniConfRoot::Iter::next()
{
    return hackit.next();
}


UniConfKey UniConfRoot::Iter::key() const
{
    return UniConfKey(hackit());
}



/***** UniConfInfoTree *****/

UniConfInfoTree::UniConfInfoTree(UniConfInfoTree *parent,
    const UniConfKey &key) :
    UniConfTree<UniConfInfoTree>(parent, key), generator(NULL)
{
}


UniConfInfoTree::~UniConfInfoTree()
{
}


UniConfInfoTree *UniConfInfoTree::findnearest(const UniConfKey &key,
    int &split)
{
    split = 0;
    UniConfInfoTree *node = this;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); ++split)
    {
        UniConfInfoTree *next = node->findchild(it());
        if (! next)
            break;
        node = next;
    }
    return node;
}


UniConfInfoTree *UniConfInfoTree::findormake(const UniConfKey &key)
{
    UniConfInfoTree *node = this;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); )
    {
        UniConfInfoTree *prev = node;
        node = prev->findchild(it());
        if (! node)
            node = new UniConfInfoTree(prev, it());
    }
    return node;
}



/***** UniConfInfoTree::NodeIter *****/

UniConfInfoTree::NodeIter::NodeIter(UniConfInfoTree &root,
    const UniConfKey &key) :
    xkey(key)
{
    bestnode = root.findnearest(key, bestsplit);
}


void UniConfInfoTree::NodeIter::rewind()
{
    xnode = NULL;
}


bool UniConfInfoTree::NodeIter::next()
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



/***** UniConfInfoTree::GenIter *****/

UniConfInfoTree::GenIter::GenIter(UniConfInfoTree &root,
    const UniConfKey &key) :
    UniConfInfoTree::NodeIter(root, key)
{
}


void UniConfInfoTree::GenIter::rewind()
{
    UniConfInfoTree::NodeIter::rewind();
}


bool UniConfInfoTree::GenIter::next()
{
    while (UniConfInfoTree::NodeIter::next())
    {
        if (node()->generator)
            return true;
    }
    return false;
}
