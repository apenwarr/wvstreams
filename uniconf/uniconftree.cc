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
    const UniConfKey &key) :
    xparent(parent), xchildren(NULL), xkey(key)
{
    if (xparent)
        xparent->link(this);
}


UniConfTreeBase::~UniConfTreeBase()
{
    // this happens only after the children are deleted by our
    // typed subclass, which ensures that we do not confuse them
    // about their parentage as their destructors are invoked
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
    bool found = false;
    int slot = bsearch(key, found);
    return found ? (*xchildren)[slot] : NULL;
}


bool UniConfTreeBase::haschildren() const
{
    return xchildren && ! xchildren->isempty();
}


void UniConfTreeBase::compact()
{
    if (xchildren)
    {
        if (xchildren->isempty())
        {
            delete xchildren;
            xchildren = NULL;
        }
        else
            xchildren->compact();
    }
}


void UniConfTreeBase::link(UniConfTreeBase *node)
{
    bool found = false;
    int slot = bsearch(node->key(), found);
    assert (! found);
    if (! xchildren)
        xchildren = new Vector();
    xchildren->insert(slot, node);
}


void UniConfTreeBase::unlink(UniConfTreeBase *node)
{
    // This case occurs because of a simple optimization in
    // UniConfTree::zap() to avoid a possible quadratic time bound
    if (! xchildren)
        return;

    bool found = false;
    int slot = bsearch(node->key(), found);
    assert(found);
    xchildren->remove(slot);
}


int UniConfTreeBase::bsearch(const UniConfKey &key, bool &found) const
{
    if (! xchildren)
        return 0; // no children!
    int low = 0;
    int high = xchildren->size();
    while (low < high)
    {
        int mid = (low + high) / 2;
        int code = key.compareto((*xchildren)[mid]->key());
        if (code < 0)
            high = mid;
        else if (code > 0)
            low = mid + 1;
        else
        {
            found = true;
            return mid;
        }
    }
    return low;
}



/***** UniConfTreeBase::Iter *****/

UniConfTreeBase::Iter::Iter(UniConfTreeBase &_tree) :
    tree(_tree)
{
}


UniConfTreeBase::Iter::Iter(const Iter &other) :
    tree(other.tree), index(other.index)
{
}


void UniConfTreeBase::Iter::rewind()
{
    index = -1;
}


bool UniConfTreeBase::Iter::next()
{
    return tree.xchildren && ++index < tree.xchildren->size();
}


UniConfTreeBase *UniConfTreeBase::Iter::ptr() const
{
    return (*tree.xchildren)[index];
}
