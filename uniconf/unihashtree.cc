/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConf low-level tree storage abstraction.
 */
#include "unihashtree.h"
#include "assert.h"


UniHashTreeBase::UniHashTreeBase(UniHashTreeBase *parent, 
    const UniConfKey &key) :
    xkey(key)
{
    xparent = parent;
    xchildren = NULL;
    
    if (xparent)
        xparent->link(this);
}


UniHashTreeBase::~UniHashTreeBase()
{
    if (xchildren)
    {
        Container *oldchildren = xchildren;
        xchildren = NULL;

        delete oldchildren;
    } 

    // This happens only after the children are deleted by our
    // subclass.  This ensures that we do not confuse them
    // about their parentage as their destructors are invoked
    // The xchildren vector is destroyed by the subclass!
    if (xparent)
        xparent->unlink(this);
}


void UniHashTreeBase::_setparent(UniHashTreeBase *parent)
{
    if (xparent == parent)
        return;
    if (xparent)
        xparent->unlink(this);
    xparent = parent;
    if (xparent)
        xparent->link(this);
}


UniHashTreeBase *UniHashTreeBase::_root() const
{
    const UniHashTreeBase *node = this;
    while (node->xparent)
        node = node->xparent;
    return const_cast<UniHashTreeBase*>(node);
}


UniConfKey UniHashTreeBase::_fullkey(const UniHashTreeBase *ancestor) const
{
    UniConfKey result;
    if (ancestor)
    {
        const UniHashTreeBase *node = this;
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
        const UniHashTreeBase *node = this;
        while (node->xparent)
        {
            result.prepend(node->key());
            node = node->xparent;
        }
    }
    return result;
}


UniHashTreeBase *UniHashTreeBase::_find(const UniConfKey &key) const
{
    const UniHashTreeBase *node = this;
    UniConfKey::Iter it(key);
    it.rewind();
    while (it.next())
    {
        node = node->_findchild(it());
        if (!node)
            break;
    }
    return const_cast<UniHashTreeBase*>(node);
}

UniHashTreeBase *UniHashTreeBase::_findchild(const UniConfKey &key) const
{
    if (key.isempty())
        return const_cast<UniHashTreeBase*>(this);

    return xchildren ? (*xchildren)[key] : NULL;
}


bool UniHashTreeBase::haschildren() const
{
    return xchildren && !xchildren->isempty();
}

void UniHashTreeBase::link(UniHashTreeBase *node)
{
    if (!xchildren)
        xchildren = new Container();

    xchildren->add(node);
}

void UniHashTreeBase::unlink(UniHashTreeBase *node)
{
    if (!xchildren)
        return;

    xchildren->remove(node);
    if (xchildren->count() == 0)
        delete xchildren;

    xchildren = NULL;
}
