/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * UniConf low-level tree storage abstraction.
 */
#ifndef __UNICONFTREE_H
#define __UNICONFTREE_H

#include "uniconfpair.h"
#include "wvlink.h"

/**
 * FIXME: document me fully after refactoring
 */
class UniConfTree : public UniConfPair
{
    UniConfTree *xparent; /*!< the parent of this subtree */
    UniConfPairDict *xchildren; /*!< the children of this node */
    
public:
    UniConfTree(UniConfTree *parent, const UniConfKey &key,
        WvStringParm value);
    ~UniConfTree();

    inline UniConfTree *parent() const
        { return xparent; }

    UniConfTree *find(const UniConfKey &key);
    UniConfTree *findormake(const UniConfKey &key);
    void remove(const UniConfKey &key);
    bool haschildren() const;

    class Iter;
    friend class UniConfTree::Iter;

protected:
    void link(UniConfTree *node);
    void unlink(UniConfTree *node);
    UniConfTree *findchild(const UniConfKey &key) const;
};

/**
 * An iterator that walks over all elements on one level
 * of the tree.
 */
class UniConfTree::Iter
{
    UniConfTree &tree;
    UniConfPairDict::Iter it;

public:
    Iter(UniConfTree &tree);

    void rewind();

    inline bool next()
    {
        return it.next();
    }
    inline UniConfTree *ptr() const
    {
        return static_cast<UniConfTree*>(it.ptr());
    }
    WvIterStuff(UniConfTree);
};

#endif //__UNICONFTREE_H
