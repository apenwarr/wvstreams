/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConf low-level tree storage abstraction.
 */
#ifndef __UNICONFTREE_H
#define __UNICONFTREE_H

#include "uniconfkey.h"
#include "wvvector.h"

/**
 * UniConfTreeBase the common base implementation for UniConfTree.
 * @see UniConfTree
 */
class UniConfTreeBase
{
protected:
    typedef WvVector<UniConfTreeBase> Vector;

    UniConfTreeBase *xparent; /*!< the parent of this subtree */
    Vector *xchildren; /*!< the ordered vector of children */
    UniConfKey xkey;   /*!< the name of this entry */

    UniConfTreeBase(UniConfTreeBase *parent, const UniConfKey &key);
    
public:
    ~UniConfTreeBase();

protected:
    void _setparent(UniConfTreeBase *parent);
    
    UniConfTreeBase *_root() const;
    UniConfKey _fullkey(const UniConfTreeBase *ancestor = NULL) const;
    UniConfTreeBase *_find(const UniConfKey &key) const;
    UniConfTreeBase *_findchild(const UniConfKey &key) const;

    class Iter : public Vector::Iter
    {
    public:
	Iter(UniConfTreeBase &b) : Vector::Iter(b.xchildren)
	    { }
    };
    friend class Iter;
    
public:
    /** Returns the key field. */
    const UniConfKey &key() const
        { return xkey; }

    /** Returns true if the node has children. */
    bool haschildren() const;

    /** Compacts the tree storage to minimize its footprint. */
    void compact();

private:
    /** Called by a child to link itself to this node. */
    void link(UniConfTreeBase *node);

    /** Called by a child to unlink itself from this node. */
    void unlink(UniConfTreeBase *node);

    /**
     * Performs a binary search and returns the slot in which a
     * child would reside if it belonged to the sequence.  Returns a negative
     * number if not found.
     */
    int bsearch(const UniConfKey &key, bool &found) const;
};


/**
 * A recursively composed dictionary for ordered tree-structured
 * data indexed by UniConfKey with logarithmic lookup time for
 * each 1-level search, linear insertion and removal.
 *
 * This container maintains a sorted vector of children which
 * greatly facilitates tree comparison and merging.  The
 * underlying collection guarantees fast lookups, but insertion
 * and removal may be quite slow.  If this becomes a problem,
 * then a different (more complex) data structure should be used,
 * such as a binary search tree.  However, for the moment, the
 * use of a vector keeps down memory footprint.
 *
 * Someday this could be further abstracted into a generic WvTreeDict.
 */
template<class T>
class UniConfTree : public UniConfTreeBase
{
public:
    /** Creates a node (and links it to a subtree, if parent is non-NULL) */
    UniConfTree(T *parent, const UniConfKey &key)
        : UniConfTreeBase(parent, key)
        { }

    /** Destroy this node's contents and children. */
    ~UniConfTree()
        { zap(); }

    /** Returns a pointer to the parent node, or NULL if there is none. */
    T *parent() const
        { return static_cast<T*>(xparent); }

    /** Reparents this node. */
    void setparent(T *parent)
        { UniConfTreeBase::_setparent(parent); }
    
    /** Returns a pointer to the root node of the tree. */
    T *root() const
        { return static_cast<T*>(UniConfTreeBase::_root()); }
    
    /**
     * Returns full path of this node relative to an ancestor
     * (or the root, if ancestor is NULL).
     */
    UniConfKey fullkey(const T *ancestor = NULL) const
        { return UniConfTreeBase::_fullkey(ancestor); }

    /**
     * Finds the sub-node with the specified key.
     * If key.isempty(), returns this node.
     */
    T *find(const UniConfKey &key) const
        { return static_cast<T*>(UniConfTreeBase::_find(key)); }
    
    /**
     * Finds the direct child node with the specified key.
     *
     * If key.numsegments() == 1, then performs the same task
     * as find(key), but a little faster.  Otherwise returns NULL.
     */
    T *findchild(const UniConfKey &key) const
        { return static_cast<T*>(UniConfTreeBase::_findchild(key)); }

    /**
     * Removes the node for the specified key from the tree
     * and deletes it along with any of its children.
     *
     * If the key is UniConfKey::EMPTY, deletes this object.
     */
    void remove(const UniConfKey &key)
        { delete find(key); }
    
    /** Removes and deletes all children of this node. */
    void zap()
    {
        if (!xchildren)
            return;
	xchildren->zap();
        delete xchildren;
	xchildren = NULL;
    }

    /**
     * An iterator that walks over all elements on one level of a
     * UniConfTree.
     */
    class Iter : public UniConfTreeBase::Iter
    {
    public:
        /** Creates an iterator over the specified tree. */
        Iter(T &tree) : UniConfTreeBase::Iter(tree)
	    { }

        /** Returns a pointer to the current node. */
        T *ptr() const
            { return static_cast<T*>(UniConfTreeBase::Iter::ptr()); }
        WvIterStuff(T);
    };
};


/**
 * A plain UniConfTree that holds keys and values.
 */
class UniConfValueTree : public UniConfTree<UniConfValueTree>
{
    WvString xvalue;  /*!< the value of this entry */
    
public:
    UniConfValueTree(UniConfValueTree *parent,
        const UniConfKey &key, WvStringParm value) :
        UniConfTree<UniConfValueTree>(parent, key), xvalue(value)
    {
    }
    
    /** Returns the value field. */
    const WvString &value()
        { return xvalue; }

    /** Sets the value field. */
    void setvalue(WvStringParm value)
        { xvalue = value; }
};


#endif // __UNICONFTREE_H
