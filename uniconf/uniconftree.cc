/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * UniConf low-level tree storage abstraction.
 */\
#include "uniconftree.h"
#include "assert.h"


/***** UniConfTreeBase *****/

UniConfTreeBase::UniConfTreeBase(UniConfTreeBase *parent,
    const UniConfKey &key, WvStringParm value) :
    UniConfPair(key, value),
    xparent(parent), xchildren(NULL)
{
    if (xparent)
        xparent->link(this);
}


UniConfTreeBase::~UniConfTreeBase()
{
    // this happens only after the children are deleted
    // by our subclass, which ensures that we do not confuse
    // them about their parentage
    if (xparent)
        xparent->unlink(this);
}


void UniConfTreeBase::_setparent(UniConfTreeBase *parent)
{
    if (xparent == parent)
        return;
    if (xparent)
        xparent->unlink(this);
    xparent = parent;
    if (xparent)
        xparent->link(this);
}


UniConfTreeBase *UniConfTreeBase::_root() const
{
    const UniConfTreeBase *node = this;
    while (node->xparent)
        node = node->xparent;
    return const_cast<UniConfTreeBase*>(node);
}


bool UniConfTreeBase::haschildren() const
{
    return xchildren && ! xchildren->isempty();
}


UniConfKey UniConfTreeBase::_fullkey(
    const UniConfTreeBase *ancestor) const
{
    UniConfKey result;
    if (ancestor)
    {
        const UniConfTreeBase *node = this;
        while (node != ancestor)
        {
            result.prepend(node->key());
            node = node->xparent;
            assert(node != NULL ||
                ! "ancestor was not a node in the tree");
        }
    }
    else
    {
        const UniConfTreeBase *node = this;
        while (node->xparent)
        {
            result.prepend(node->key());
            node = node->xparent;
        }
    }
    return result;
}


UniConfTreeBase *UniConfTreeBase::_find(
    const UniConfKey &key) const
{
    const UniConfTreeBase *node = this;
    UniConfKey::Iter it(key);
    it.rewind();
    while (it.next())
    {
        node = node->_findchild(it());
        if (! node)
            break;
    }
    return const_cast<UniConfTreeBase*>(node);
}


UniConfTreeBase *UniConfTreeBase::_findchild(
    const UniConfKey &key) const
{
    if (! xchildren)
        return NULL;
    return (*xchildren)[key];
}


void UniConfTreeBase::link(UniConfTreeBase *node)
{
    if (! xchildren)
        xchildren = new UniConfTreeBaseDict(13); // FIXME: should not fix size
    xchildren->add(node, true);
}


void UniConfTreeBase::unlink(UniConfTreeBase *node)
{
    if (xchildren)
        xchildren->remove(node);
}



/***** UniConfTreeBase::Iter *****/

UniConfTreeBase::Iter::Iter(UniConfTreeBase &_tree) :
    tree(_tree), it(* reinterpret_cast<UniConfTreeBaseDict*>(
        & null_UniConfPairDict))
{
}


UniConfTreeBase::Iter::Iter(const Iter &other) :
    tree(other.tree), it(other.it)
{
}


void UniConfTreeBase::Iter::rewind()
{
    // an awful hack so that we can rewind an iterator after
    // things have been added to a tree and discover new children
    it.tbl = tree.xchildren ? tree.xchildren :
        reinterpret_cast<UniConfTreeBaseDict*>(& null_UniConfPairDict);
    it.rewind();
}
