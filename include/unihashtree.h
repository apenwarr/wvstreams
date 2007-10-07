/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConf low-level tree storage abstraction.
 */
#ifndef __UNIHASHTREE_H
#define __UNIHASHTREE_H

#include "uniconfkey.h"
#include "wvtr1.h"
#include "wvscatterhash.h"

class UniHashTreeBase;

// parameters: a node (won't be NULL), userdata
typedef wv::function<void(const UniHashTreeBase*,
			  void*)> UniHashTreeBaseVisitor;
// parameters: 1st node (may be NULL), 2nd node (may be NULL), userdata
typedef wv::function<bool(const UniHashTreeBase*, 
			  const UniHashTreeBase*,
			  void*)> UniHashTreeBaseComparator;

class UniHashTreeBase
{
protected:
    UniConfKey xkey;   /*!< the name of this entry */
    UniHashTreeBase *xparent; /*!< the parent of this subtree */

    struct Accessor
    {
        static const UniConfKey *get_key(const UniHashTreeBase *obj)
            { return &obj->key(); }
    };

    typedef WvScatterHash<UniHashTreeBase, UniConfKey, Accessor> Container;
    typedef UniHashTreeBaseVisitor BaseVisitor;
    typedef UniHashTreeBaseComparator BaseComparator;
    Container *xchildren; /*!< the hash table of children */

    UniHashTreeBase(UniHashTreeBase *parent, const UniConfKey &key);

public:
    ~UniHashTreeBase();

protected:
    void _setparent(UniHashTreeBase *parent);
    
    UniHashTreeBase *_root() const;
    UniConfKey _fullkey(const UniHashTreeBase *ancestor = NULL) const;
    UniHashTreeBase *_find(const UniConfKey &key) const;
    UniHashTreeBase *_findchild(const UniConfKey &key) const;

    static bool _recursivecompare(
        const UniHashTreeBase *a, const UniHashTreeBase *b,
        const UniHashTreeBaseComparator &comparator, void *userdata);

    static void _recursive_unsorted_visit(
        const UniHashTreeBase *a,
        const UniHashTreeBaseVisitor &visitor, void *userdata,
	bool preorder, bool postorder);

public:
    class Iter : public Container::Iter
    {
    public:
        Iter(UniHashTreeBase &b) : Container::Iter(*b.xchildren) { }
    };
    friend class Iter;

    /** Returns the key field. */
    const UniConfKey &key() const
        { return xkey; }

    /** Returns true if the node has children. */
    bool haschildren() const;

private:
    /** Called by a child to link itself to this node. */
    void link(UniHashTreeBase *node);

    /** Called by a child to unlink itself from this node. */
    void unlink(UniHashTreeBase *node);
};

#endif //__UNIHASHTREE_H
