/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConf is the new, improved, hierarchical version of WvConf.  It stores
 * strings in a hierarchy and can load/save them from "various places."
 * 
 * See uniconf.h.
 */
#include "uniconf.h"
#include "uniconfnull.h"
#include "wvstream.h"
#include "wvstringtable.h"
#include "uniconfiter.h"
#include <assert.h>

/***** UniConf *****/

bool UniConf::haschildren() const
{
    return xmanager->haschildren(xfullkey);
}


bool UniConf::exists() const
{
    return xmanager->exists(xfullkey);
}


WvString UniConf::get(WvStringParm defvalue) const
{
    WvString value = xmanager->get(xfullkey);
    if (value.isnull())
        return defvalue;
    return value;
}


int UniConf::getint(int defvalue) const
{
    // also recognize bool strings as integers
    const char *strs[] = {
        "true", "yes", "on", "enabled",
        "false", "no", "off", "disabled"
    };
    const size_t numtruestrs = 4;

    WvString value = get();
    if (! value)
    {
        // only if non-null and non-empty
        char *end;
        int num = strtol(value.cstr(), & end, 0);
        if (*end)
            return num; // was a valid integer
    }

    // try to recognize a special string
    for (size_t i = 0; i < sizeof(strs) / sizeof(const char*); ++i)
        if (strcasecmp(value, strs[i]) == 0)
            return i < numtruestrs;

    return defvalue;
}


bool UniConf::set(WvStringParm value) const
{
    return xmanager->set(xfullkey, value);
}


bool UniConf::setint(int value) const
{
    return xmanager->set(xfullkey, WvString(value));
}


bool UniConf::zap() const
{
    return xmanager->zap(xfullkey);
}


bool UniConf::refresh(UniConf::Depth depth) const
{
    return xmanager->refresh(xfullkey, depth);
}


bool UniConf::commit(UniConf::Depth depth) const
{
    return xmanager->commit(xfullkey, depth);
}


UniConfGen *UniConf::mount(const UniConfLocation &location,
    bool refresh) const
{
    return xmanager->mount(xfullkey, location, refresh);
}


void UniConf::mountgen(UniConfGen *gen, bool refresh) const
{
    xmanager->mountgen(xfullkey, gen, refresh);
}


void UniConf::unmount() const
{
    xmanager->unmount(xfullkey);
}


bool UniConf::isok() const
{
    return xmanager->isok(xfullkey);
}


bool UniConf::ismountpoint() const
{
    return xmanager->ismountpoint(xfullkey);
}


void UniConf::dump(WvStream &stream, bool everything) const
{
    UniConf::RecursiveIter it(*this);
    for (it.rewind(); it.next(); )
    {
        WvString value(it->get());
        if (everything || !! value)
            stream.print("%s = %s\n", it->fullkey(), value);
    }
}



/***** UniConf::Iter *****/

UniConf::Iter::Iter(const UniConf &root) :
    UniConfRoot::Iter(*root.xmanager, root.xfullkey),
    xroot(root), current(NULL)
{
}


bool UniConf::Iter::next()
{
    if (UniConfRoot::Iter::next())
    {
        current = xroot[key()];
        return true;
    }
    return false;
}



/***** UniConf::RecursiveIter *****/

UniConf::RecursiveIter::RecursiveIter(const UniConf &root,
    UniConf::Depth depth) :
    xroot(root), top(root), depth(depth), current(NULL)
{
}


void UniConf::RecursiveIter::rewind()
{
    itlist.zap();
    first = false;
    switch (depth)
    {
        case UniConf::ZERO:
            first = true;
            break;

        case UniConf::ONE:
        case UniConf::INFINITE:
            first = true;
            // fall through

        case UniConf::CHILDREN:
        case UniConf::DESCENDENTS:
            itlist.append(& top, false);
            top.rewind();
            break;
    }
}


bool UniConf::RecursiveIter::next()
{
    if (first)
    {
        first = false;
        current = xroot;
        return true;
    }

    IterList::Iter itlistit(itlist);
    for (itlistit.rewind(); itlistit.next(); )
    {
        UniConf::Iter &it = itlistit();
        if (it.next())
        {
            current = *it;
            if ((depth == UniConf::INFINITE ||
                depth == UniConf::DESCENDENTS) &&
                current.haschildren())
            {
                UniConf::Iter *subit = new UniConf::Iter(current);
                subit->rewind();
                itlist.prepend(subit, true);
            }
            return true;
        }
        itlistit.xunlink();
    }
    return false;
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



/***** UniConfRoot *****/

UniConfRoot::UniConfRoot() :
    root(NULL, UniConfKey::EMPTY), streamlist(NULL)
{
}


UniConfRoot::~UniConfRoot()
{
}


WvString UniConfRoot::get(const UniConfKey &key, WvStringParm defvalue)
{
    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        WvString result = gen->get(it.tail());
        if (! result.isnull())
            return result;
    }
    return defvalue;
}


bool UniConfRoot::set(const UniConfKey &key, WvStringParm value)
{
    UniConfInfoTree::GenIter it(root, key);
    // locate the generator that defines the key, if any
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->exists(it.tail()))
            return gen->set(it.tail(), value);
    }
    
    // otherwise, set the key on the first generator that works
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->set(it.tail(), value))
            return true;
    }
    return false;
}


bool UniConfRoot::zap(const UniConfKey &key)
{
    bool success = true;
    UniConfInfoTree::GenIter it(root, key);
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
    UniConfInfoTree *node = root.find(key);
    if (node)
        return true;
        
    UniConfInfoTree::GenIter it(root, key);
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
    UniConfInfoTree *node = root.find(key);
    if (node && node->haschildren())
        return true;

    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (gen->haschildren(it.tail()))
            return true;
    }
    return false;
}


bool UniConfRoot::refresh(const UniConfKey &key, UniConf::Depth depth)
{
    return dorecursive(genrefreshfunc, key, depth);
}


bool UniConfRoot::genrefreshfunc(UniConfGen *gen,
    const UniConfKey &key, UniConf::Depth depth)
{
    return gen->refresh(key, depth);
}


bool UniConfRoot::commit(const UniConfKey &key, UniConf::Depth depth)
{
    return dorecursive(gencommitfunc, key, depth);
}


bool UniConfRoot::gencommitfunc(UniConfGen *gen,
    const UniConfKey &key, UniConf::Depth depth)
{
    return gen->commit(key, depth);
}


bool UniConfRoot::dorecursive(GenFunc func, const UniConfKey &key,
    UniConf::Depth depth)
{
    // do containing generators
    bool success = true;
    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        if (! func(gen, it.tail(), depth))
            success = false;
    }

    // do recursive
    if (depth != UniConf::ZERO)
    {
        UniConfInfoTree *node = root.find(key);
        if (node && ! dorecursivehelper(func, node, depth))
            success = false;
    }
    return success;
}


bool UniConfRoot::dorecursivehelper(GenFunc func,
    UniConfInfoTree *node, UniConf::Depth depth)
{
    // determine depth for next step
    switch (depth)
    {
        case UniConf::ZERO:
            assert(false);
            return true;

        case UniConf::ONE:
        case UniConf::CHILDREN:
            depth = UniConf::ZERO;
            break;

        case UniConf::DESCENDENTS:
            depth = UniConf::INFINITE;
        case UniConf::INFINITE:
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
        if (depth != UniConf::ZERO)
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
    // FIXME: must iterator over all existing generators
}


void UniConfRoot::detach(WvStreamList *_streamlist)
{
    assert(streamlist != NULL);
    assert(streamlist == _streamlist);
    streamlist = NULL;
    // FIXME: must iterator over all existing generators
}


UniConfGen *UniConfRoot::mount(const UniConfKey &key,
    const UniConfLocation &location, bool refresh)
{
    UniConfGen *gen = UniConfGenFactoryRegistry::instance()->
        newgen(location);
    mountgen(key, gen, refresh);
    return gen;
}


void UniConfRoot::mountgen(const UniConfKey &key,
    UniConfGen *gen, bool refresh)
{
    unmount(key);
    UniConfInfoTree *node = root.findormake(key);
    node->generator = gen;
    if (refresh)
        gen->refresh(UniConfKey::EMPTY, UniConf::INFINITE);
}


void UniConfRoot::unmount(const UniConfKey &key)
{
    UniConfInfoTree *node = root.find(key);
    if (node)
    {
        delete node->generator;
        node->generator = NULL;
    }
}


bool UniConfRoot::isok(const UniConfKey &key)
{
    UniConfInfoTree::GenIter it(root, key);
    for (it.rewind(); it.next(); )
    {
        UniConfGen *gen = it.ptr();
        return gen->isok();
    }
    return true;
}


bool UniConfRoot::ismountpoint(const UniConfKey &key)
{
    UniConfInfoTree *node = root.find(key);
    return node != NULL && node->generator;
}


void UniConfRoot::prune(UniConfInfoTree *node)
{
    while (node != & root && ! node->isessential())
    {
        UniConfInfoTree *next = node->parent();
        delete node;
        node = next;
    }
}



/***** UniConfRoot::Iter *****/

UniConfRoot::Iter::Iter(UniConfRoot &root, const UniConfKey &key) :
    xroot(& root), xkey(key), genit(root.root, key),
    hack(71), hackit(hack)
{
}


void UniConfRoot::Iter::rewind()
{
    hack.zap();

    // add mountpoint nodes
    UniConfInfoTree *node = xroot->root.find(xkey);
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
