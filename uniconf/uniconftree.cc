/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConf low-level tree storage abstraction.
 */
#include "uniconftree.h"
#include "assert.h"


UniConfTreeBase::UniConfTreeBase(UniConfTreeBase *parent, 
    const UniConfKey &key) :
    xkey(key)
{
    xparent = parent;
    xchildren = NULL;
    
    if (xparent)
        xparent->link(this);
}


UniConfTreeBase::~UniConfTreeBase()
{
    // This happens only after the children are deleted by our
    // subclass.  This ensures that we do not confuse them
    // about their parentage as their destructors are invoked
    // The xchildren vector is destroyed by the subclass!
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


UniConfKey UniConfTreeBase::_fullkey(const UniConfTreeBase *ancestor) const
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


UniConfTreeBase *UniConfTreeBase::_find(const UniConfKey &key) const
{
    const UniConfTreeBase *node = this;
    UniConfKey::Iter it(key);
    it.rewind();
    while (it.next())
    {
        node = node->_findchild(it());
        if (!node)
            break;
    }
    return const_cast<UniConfTreeBase*>(node);
}


UniConfTreeBase *UniConfTreeBase::_findchild(const UniConfKey &key) const
{
    bool found = false;
    int slot = bsearch(key, found);
    return found ? (*xchildren)[slot] : NULL;
}


bool UniConfTreeBase::haschildren() const
{
    return xchildren && !xchildren->isempty();
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


void UniConfTreeBase::link(UniConfTreeBase *child)
{
    bool found = false;
    int slot = bsearch(child->key(), found);
    assert (!found);
    if (!xchildren)
        xchildren = new Container(true);
    xchildren->insert(slot, child);
}


void UniConfTreeBase::unlink(UniConfTreeBase *node)
{
    // This case occurs because of a simple optimization in
    // UniConfTree::zap() to avoid a possible quadratic time bound
    if (!xchildren)
        return;

    bool found = false;
    int slot = bsearch(node->key(), found);
    if (found)
	xchildren->remove(slot, true /*never_delete*/);
}


int UniConfTreeBase::bsearch(const UniConfKey &key, bool &found) const
{
    if (!xchildren)
        return 0; // no children!
    int low = 0;
    int high = xchildren->count();
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


void UniConfTreeBase::_recursivecompare(
    const UniConfTreeBase *a, const UniConfTreeBase *b,
    const UniConfTreeBaseComparator &comparator, void *userdata)
{
    // don't bother comparing subtree if this returns false
    if (! comparator(a, b, userdata))
        return;
    
    // begin iteration sequence
    Iter ait(*const_cast<UniConfTreeBase*>(a));
    if (a != NULL)
    {
        ait.rewind();
        if (ait.next())
            a = ait.ptr();
	else
	    a = NULL;
    }
    Iter bit(*const_cast<UniConfTreeBase*>(b));
    if (b != NULL)
    {
        bit.rewind();
        if (bit.next())
            b = bit.ptr();
	else
	    b = NULL;
    }

    // loop
    while (a != NULL && b != NULL)
    {
        int order = a->key().compareto(b->key());
        if (order < 0)
        {
            _recursivecompare(a, NULL, comparator, userdata);
            a = ait.next() ? ait.ptr() : NULL;
        }
        else if (order == 0)
        {
	    // keys are equal
            _recursivecompare(a, b, comparator, userdata);
            a = ait.next() ? ait.ptr() : NULL;
            b = bit.next() ? bit.ptr() : NULL;
        }
        else
        {
            _recursivecompare(NULL, b, comparator, userdata);
            b = bit.next() ? bit.ptr() : NULL;
        }
    }
    while (a != NULL)
    {
        _recursivecompare(a, NULL, comparator, userdata);
        a = ait.next() ? ait.ptr() : NULL;
    }
    while (b != NULL)
    {
        _recursivecompare(NULL, b, comparator, userdata);
        b = bit.next() ? bit.ptr() : NULL;
    }
}
