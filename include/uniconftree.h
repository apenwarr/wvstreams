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
#include "wvcallback.h"

class UniConfTreeBase;

// parameters: 1st node (may be NULL), 2nd node (may be NULL), userdata
DeclareWvCallback(3, bool, UniConfTreeBaseComparator,
    const UniConfTreeBase *, const UniConfTreeBase *, void *);

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

    static void _recursivecompare(
        const UniConfTreeBase *a, const UniConfTreeBase *b,
        const UniConfTreeBaseComparator &comparator, void *userdata);
    
    friend class Iter : public Vector::Iter
    {
    public:
	Iter(UniConfTreeBase &b) : Vector::Iter(b.xchildren)
	    { }
    };
    
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
 *
 * "Sub" is the name of the concrete subclass of UniConfTree
 */
template<class Sub>
class UniConfTree : public UniConfTreeBase
{
protected:
    typedef WvVector<Sub> SubVector;
    
public:
    DeclareWvCallback(3, bool, Comparator,
        const Sub *, const Sub *, void *);

    /** Creates a node and links it to a subtree, if parent is non-NULL */
    UniConfTree(Sub *parent, const UniConfKey &key) :
        UniConfTreeBase(parent, key)
        { }

    /** Destroy this node's contents and children. */
    ~UniConfTree()
        { zap(); }

    /** Returns a pointer to the parent node, or NULL if there is none. */
    Sub *parent() const
        { return static_cast<Sub*>(xparent); }

    /** Reparents this node. */
    void setparent(Sub *parent)
        { UniConfTreeBase::_setparent(parent); }
    
    /** Returns a pointer to the root node of the tree. */
    Sub *root() const
        { return static_cast<Sub*>(UniConfTreeBase::_root()); }
    
    /**
     * Returns full path of this node relative to an ancestor.
     * If ancestor is NULL, returns the root.
     */
    UniConfKey fullkey(const Sub *ancestor = NULL) const
        { return UniConfTreeBase::_fullkey(ancestor); }

    /**
     * Finds the sub-node with the specified key.
     * If key.isempty(), returns this node.
     */
    Sub *find(const UniConfKey &key) const
        { return static_cast<Sub*>(UniConfTreeBase::_find(key)); }
    
    /**
     * Finds the direct child node with the specified key.
     *
     * If key.numsegments() == 1, then performs the same task
     * as find(key), but a little faster.  Otherwise returns NULL.
     */
    Sub *findchild(const UniConfKey &key) const
        { return static_cast<Sub*>(UniConfTreeBase::_findchild(key)); }

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
        // set xchildren to NULL so that the zap() will happen faster
        // otherwise, each child will attempt to unlink itself uselessly
        SubVector *oldchildren = reinterpret_cast<SubVector*>(xchildren);
        xchildren = NULL;
        delete oldchildren; // deletes all children
    }

    /**
     * Compares this tree with another using the specified comparator
     * function.
     * Comparison of a subtree ends when the comparator returns false.
     * "comparator" is the value compare function
     * "userdata" is userdata for the compare function
     * Returns: true if the comparison function returned true each time
     */
    void compare(const Sub *other, const Comparator &comparator,
        void *userdata)
    {
        _recursivecompare(this, other, reinterpret_cast<
            const UniConfTreeBaseComparator&>(comparator), userdata);
    }

    /**
     * An iterator that walks over all elements on one level of a
     * UniConfTree.
     */
    class Iter : public UniConfTreeBase::Iter
    {
    public:
        /** Creates an iterator over the specified tree. */
        Iter(Sub &tree) : UniConfTreeBase::Iter(tree)
	    { }

        /** Returns a pointer to the current node. */
        Sub *ptr() const
            { return static_cast<Sub*>(UniConfTreeBase::Iter::ptr()); }
        WvIterStuff(Sub);
    };
};


/** A plain UniConfTree that holds keys and values. */
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
    const WvString &value() const
        { return xvalue; }

    /** Sets the value field. */
    void setvalue(WvStringParm value)
        { xvalue = value; }
};


#endif // __UNICONFTREE_H
