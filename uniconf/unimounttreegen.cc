/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a UniConfGen that manages a tree of UniConfGen instances.
 */
#include "unimounttreegen.h"
#include "wvmoniker.h"

/***** UniMountTreeGen *****/

UniMountTreeGen::UniMountTreeGen()
{
    mounts = new UniMountTree(NULL, UniConfKey::EMPTY);
}


UniMountTreeGen::~UniMountTreeGen()
{
    // destroys all generators
    delete mounts;
}


WvString UniMountTreeGen::get(const UniConfKey &key)
{
    // consult the generators
    hold_delta();
    UniMountTree::GenIter it(*mounts, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        WvString result = gen->get(it.tail());
        if (!result.isnull())
        {
            unhold_delta();
            return result;
        }
    }
    unhold_delta();
    
    // ensure key exists if it is in the path of a mountpoint
    UniMountTree *node = mounts->find(key);
    if (node)
        return ""; // fake read-only key not provided by anyone

    // no matches
    return WvString::null;
}


void UniMountTreeGen::set(const UniConfKey &key, WvStringParm value)
{
    // update the generator that defines the key, if any
    UniConfKey mountpoint;
    UniConfGen *provider = whichmount(key, &mountpoint);
    if (provider)
        provider->set(mountpoint, value);
}


bool UniMountTreeGen::exists(const UniConfKey &key)
{
    // ensure key exists if it is in the path of a mountpoint
    UniMountTree *node = mounts->find(key);
    if (node)
        return true;
    
    // otherwise consult the generators
    hold_delta();
    UniMountTree::GenIter it(*mounts, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->exists(it.tail()))
        {
            unhold_delta();
            return true;
        }
    }
    unhold_delta();

    // no match
    return false;
}


bool UniMountTreeGen::haschildren(const UniConfKey &key)
{
    hold_delta();
    UniMountTree *node = mounts->find(key);
    bool result = node && node->haschildren();
    if (! result)
    {
        UniMountTree::GenIter it(*mounts, key);
        for (it.rewind(); it.next(); )
        {
            UniConfGen *gen = it.ptr();
            if (gen->haschildren(it.tail()))
            {
                result = true;
                break;
            }
        }
    }
    unhold_delta();
    return result;
}


bool UniMountTreeGen::refresh()
{
    hold_delta();
    bool result = true;

    UniConfGenList::Iter i(mounts->generators);
    for (i.rewind(); i.next();)
        result = result && i->refresh();

    unhold_delta();
    return result;
}


void UniMountTreeGen::commit()
{
    hold_delta();

    UniConfGenList::Iter i(mounts->generators);
    for (i.rewind(); i.next();)
        i->commit();

    unhold_delta();
}


UniConfGen *UniMountTreeGen::mount(const UniConfKey &key,
    WvStringParm moniker, bool refresh)
{
    UniConfGen *gen = wvcreate<UniConfGen>(moniker);
    if (gen)
        mountgen(key, gen, refresh); // assume always succeeds for now
    return gen;
}


UniConfGen *UniMountTreeGen::mountgen(const UniConfKey &key,
    UniConfGen *gen, bool refresh)
{
    UniMountTree *node = mounts->findormake(key);
    node->generators.append(gen, true);
    
    hold_delta();
    
    gen->setcallback(wvcallback(UniConfGenCallback, *this,
        UniMountTreeGen::gencallback), node);
    if (gen && refresh)
        gen->refresh();
    
    unhold_delta();
    return gen;
}


void UniMountTreeGen::unmount(const UniConfKey &key,
    UniConfGen *gen, bool commit)
{
    UniMountTree *node = mounts->find(key);
    if (!node)
        return;

    UniConfGenList::Iter genit(node->generators);
    if (! genit.find(gen))
        return;

    hold_delta();
    
    if (commit)
        gen->commit();
    gen->setcallback(NULL, NULL);

    node->generators.unlink(gen);
    
    unhold_delta();
}


UniConfGen *UniMountTreeGen::whichmount(const UniConfKey &key,
    UniConfKey *mountpoint)
{
    hold_delta();
    
    // see if a generator acknowledges the key
    UniMountTree::GenIter it(*mounts, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->exists(it.tail()))
            goto found;
    }
    // find the generator that would be used to set the value
    it.rewind();
    if (! it.next())
    {
        unhold_delta();
        return NULL;
    }

found:
    if (mountpoint)
        *mountpoint = it.tail();
    unhold_delta();
    return it.ptr();
}


bool UniMountTreeGen::ismountpoint(const UniConfKey &key)
{
    UniMountTree *node = mounts->find(key);
    return node && ! node->generators.isempty();
}


UniMountTreeGen::Iter *UniMountTreeGen::iterator(const UniConfKey &key)
{
    return new KeyIter(*this, key);
}


void UniMountTreeGen::prune(UniMountTree *node)
{
    while (node != mounts && !node->isessential())
    {
        UniMountTree *next = node->parent();
        delete node;
        node = next;
    }
}


void UniMountTreeGen::gencallback(UniConfGen *gen,
    const UniConfKey &key, void *userdata)
{
    UniMountTree *node = static_cast<UniMountTree*>(userdata);
    delta(UniConfKey(node->fullkey(), key));
}



/***** UniMountTreeGen::KeyIter *****/

UniMountTreeGen::KeyIter::KeyIter(UniMountTreeGen &root, const UniConfKey &key) 
    : xroot(&root), xkey(key), genit(*root.mounts, key),
        hack(71), hackit(hack)
{
}


void UniMountTreeGen::KeyIter::rewind()
{
    xroot->hold_delta();
    hack.zap();

    // find nodes provided by the root of any mount points.
    // (if we want to iterate over /foo, then mounts directly on /foo/blah and
    // /foo/snoo must be included)
    UniMountTree *node = xroot->mounts->find(xkey);
    if (node)
    {
        UniMountTree::Iter nodeit(*node);
        for (nodeit.rewind(); nodeit.next(); )
            hack.add(new WvString(nodeit->key()), true);
    }

    // walk through *all* generators and add any appropriate sub-keys
    // provided by each generator.
    for (genit.rewind(); genit.next(); )
    {
        UniConfGen *gen = genit.ptr();
        UniConfGen::Iter *keyit = gen->iterator(genit.tail());
        for (keyit->rewind(); keyit->next(); )
            hack.add(new WvString(keyit->key()), true);
        delete keyit;
    }

    hackit.rewind();
    xroot->unhold_delta();
}


bool UniMountTreeGen::KeyIter::next()
{
    return hackit.next();
}


UniConfKey UniMountTreeGen::KeyIter::key() const
{
    return UniConfKey(hackit());
}



/***** UniMountTree *****/

UniMountTree::UniMountTree(UniMountTree *parent,
    const UniConfKey &key) :
    UniConfTree<UniMountTree>(parent, key)
{
}


UniMountTree::~UniMountTree()
{
}


UniMountTree *UniMountTree::findnearest(const UniConfKey &key,
    int &split)
{
    split = 0;
    UniMountTree *node = this;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); split++)
    {
        UniMountTree *next = node->findchild(it());
        if (!next)
            break;
        node = next;
    }
    return node;
}


UniMountTree *UniMountTree::findormake(const UniConfKey &key)
{
    UniMountTree *node = this;
    UniConfKey::Iter it(key);
    for (it.rewind(); it.next(); )
    {
        UniMountTree *prev = node;
        node = prev->findchild(it());
        if (!node)
            node = new UniMountTree(prev, it());
    }
    return node;
}



/***** UniMountTree::MountIter *****/

UniMountTree::MountIter::MountIter(UniMountTree &root,
    const UniConfKey &key)
    : xkey(key)
{
    bestnode = root.findnearest(key, bestsplit);
}


void UniMountTree::MountIter::rewind()
{
    xnode = NULL;
}


bool UniMountTree::MountIter::next()
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



/***** UniMountTree::GenIter *****/

UniMountTree::GenIter::GenIter(UniMountTree &root,
    const UniConfKey &key) :
    UniMountTree::MountIter(root, key),
    genit(NULL)
{
}


UniMountTree::GenIter::~GenIter()
{
    delete genit;
}


void UniMountTree::GenIter::rewind()
{
    if (genit)
    {
        delete genit;
        genit = NULL;
    }
    UniMountTree::MountIter::rewind();
}


bool UniMountTree::GenIter::next()
{
    for (;;)
    {
        if (genit && genit->next())
            return true;
        if (! UniMountTree::MountIter::next())
            return false;

        genit = new UniConfGenList::Iter(node()->generators);
        genit->rewind();
    }
    return false;
}
