/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * UniConf low-level tree storage abstraction.
 */\
#include "uniconftree.h"
#include "assert.h"

/***** UniConfTree *****/

UniConfTree::UniConfTree(UniConfTree *parent,
    const UniConfKey &key, WvStringParm value) :
    UniConfPair(key, value),
    xparent(parent), xchildren(NULL)
{
    if (xparent)
        xparent->link(this);
}


UniConfTree::~UniConfTree()
{
    delete xchildren;
}


void UniConfTree::link(UniConfTree *node)
{
    if (! xchildren)
        xchildren = new UniConfPairDict(13); // FIXME: should not fix size
    xchildren->add(node, true);
}


void UniConfTree::unlink(UniConfTree *node)
{
    assert(xchildren != NULL && node != NULL);
    xchildren->remove(node);
}


bool UniConfTree::haschildren() const
{
    return xchildren && ! xchildren->isempty();
}


UniConfTree *UniConfTree::find(const UniConfKey &key)
{
    UniConfTree *node = this;
    UniConfKey::Iter it(key);
    it.rewind();
    while (it.next())
    {
        node = node->findchild(it());
        if (! node)
            break;
    }
    return node;
}


void UniConfTree::remove(const UniConfKey &key)
{
    UniConfTree *node = this;
    UniConfKey::Iter it(key);
    it.rewind();
    while (it.next())
    {
        node = node->findchild(it());
        if (! node)
            return;
    }
    UniConfTree *prev = node->parent();
    if (prev)
        prev->unlink(node);
    else
        delete node;
}


UniConfTree *UniConfTree::findormake(const UniConfKey &key)
{
    UniConfTree *node = this;
    UniConfKey::Iter it(key);
    it.rewind();
    while (it.next())
    {
        UniConfTree *prev = node;
        node = node->findchild(it());
        if (! node)
            node = new UniConfTree(prev, it(), WvString::null);
    }
    return node;
}


UniConfTree *UniConfTree::findchild(const UniConfKey &key) const
{
    if (! xchildren)
        return NULL;
    return static_cast<UniConfTree*>((*xchildren)[key]);
}



/***** UniConfTree::Iter *****/

UniConfTree::Iter::Iter(UniConfTree &_tree) :
    tree(_tree), it(null_UniConfPairDict)
{
}


void UniConfTree::Iter::rewind()
{
    // an awful hack so that we can rewind an iterator after
    // things have been added to a tree and discover new children
    it.tbl = tree.xchildren ? tree.xchildren : & null_UniConfPairDict;
    it.rewind();
}
