/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
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
    typedef WvVector<UniConfTreeBase*,
        ShallowBlockOps<UniConfTreeBase*> > Vector;

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

    class Iter;
    friend class Iter;
    
public:
    /**
     * Returns the key field.
     * @param the key
     */
    inline const UniConfKey &key() const
        { return xkey; }

    /**
     * Returns true if the node has children.
     * @return true if the node has children
     */
    bool haschildren() const;

    /**
     * Compacts the tree storage to minimize its footprint.
     */
    void compact();

private:
    /**
     * Called by a child to link itself to this node.
     */
    void link(UniConfTreeBase *node);

    /**
     * Called by a child to unlink itself from this node.
     */
    void unlink(UniConfTreeBase *node);

    /**
     * Performs a binary search to determine the slot in which a
     * child would reside if it belonged to the sequence.
     * @param key the key to search for
     * @param found set to true if the element was found, otherwise
     *        not changed
     * @return the slot index
     */
    int bsearch(const UniConfKey &key, bool &found) const;
};


/**
 * An iterator that walks over all elements on one level of a
 * UniConfTreeBase.
 */
class UniConfTreeBase::Iter
{
    UniConfTreeBase &tree;
    int index;

public:
    Iter(UniConfTreeBase &tree);
    Iter(const Iter &other);

    void rewind();
    bool next();
    UniConfTreeBase *ptr() const;
    WvIterStuff(UniConfTreeBase);
};


/**
 * A recursively composed dictionary for ordered tree-structured
 * data indexed by UniConfKey with logarithmic lookup time for
 * each 1-level search, linear insertion and removal.
 * <p>
 * This container maintains a sorted vector of children which
 * greatly facilitates tree comparison and merging.  The
 * underlying collection guarantees fast lookups, but insertion
 * and removal may be quite slow.  If this becomes a problem,
 * then a different (more complex) data structure should be used,
 * such as a binary search tree.  However, for the moment, the
 * use of a vector keeps down memory footprint.
 * </p><p>
 * Someday this could be further abstracted into a generic WvTreeDict.
 * </p>
 *
 * @param T the name of the concrete subclass of UniConfTree
 */
template<class T>
class UniConfTree : public UniConfTreeBase
{
public:
    /**
     * Creates a node and links it to a subtree.
     * @param parent the parent node, or NULL
     * @param key the key
     */
    inline UniConfTree(T *parent, const UniConfKey &key) :
        UniConfTreeBase(parent, key)
    {
    }

    /**
     * Unlinks the node from its parent, destroys it along with
     * its contents and children.
     */
    inline ~UniConfTree()
    {
        zap();
    }

    /**
     * Returns a pointer to the parent node.
     * @return the parent, or NULL
     */
    inline T *parent() const
    {
        return static_cast<T*>(xparent);
    }

    /**
     * Reparents this node.
     * @param parent the new parent, or NULL
     */
    inline void setparent(T *parent)
    {
        UniConfTreeBase::_setparent(parent);
    }
    
    /**
     * Returns a pointer to the root node of the tree.
     * @return the root, non-NULL
     */
    inline T *root() const
    {
        return static_cast<T*>(UniConfTreeBase::_root());
    }
    
    /**
     * Returns full path of this node relative to an ancestor.
     * @param ancestor the ancestor, or NULL for the root
     * @return the path
     */
    inline UniConfKey fullkey(const T *ancestor = NULL) const
    {
        return UniConfTreeBase::_fullkey(ancestor);
    }

    /**
     * Finds the node with the specified key.
     * <p>
     * If key.isempty(), returns this node.
     * </p>
     * @param key the key
     * @return the node, or NULL
     */
    inline T *find(const UniConfKey &key) const
    {
        return static_cast<T*>(UniConfTreeBase::_find(key));
    }
    
    /**
     * Finds the direct child node with the specified key.
     * <p>
     * If key.numsegments() == 1, then performs the same task
     * as find(key), but a little faster.  Otherwise returns NULL.
     * </p>
     * @param key the key
     * @return the node, or NULL
     */
    inline T *findchild(const UniConfKey &key) const
    {
        return static_cast<T*>(UniConfTreeBase::_findchild(key));
    }

    /**
     * Removes the node for the specified key from the tree
     * and deletes it along with any of its children.
     *
     * If the key is UniConfKey::EMPTY, deletes this object.
     *
     * @param key the key
     */
    void remove(const UniConfKey &key)
    {
        delete find(key);
    }
    
    /**
     * Removes and deletes all children of this node.
     */
    void zap()
    {
        if (! xchildren)
            return;
        Vector *oldchildren = xchildren;
        xchildren = NULL;
        for (int i = 0; i < oldchildren->size(); ++i)
            delete static_cast<T*>((*oldchildren)[i]);
        delete oldchildren;
    }

    /**
     * An iterator that walks over all elements on one level of a
     * UniConfTree.
     */
    class Iter : private UniConfTreeBase::Iter
    {
    public:
        /**
         * Creates an iterator over the specified tree.
         */
        Iter(T &tree) : UniConfTreeBase::Iter(tree) { }

        /**
         * Creates a copy of another iterator that begins iteration
         * at the same position as the other left off if rewind()
         * is not called.
         */
        Iter(const Iter &other) : UniConfTreeBase::Iter(other) { }

        using UniConfTreeBase::Iter::rewind;
        using UniConfTreeBase::Iter::next;

        /**
         * Returns a pointer to the current node.
         */
        inline T *ptr() const
        {
            return static_cast<T*>(UniConfTreeBase::Iter::ptr());
        }
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
    
    /**
     * Returns the value field.
     * @param the value
     */
    inline const WvString &value()
        { return xvalue; }

    /**
     * Sets the value field.
     * @param value the new value
     */
    void setvalue(WvStringParm value)
        { xvalue = value; }
};


#endif //__UNICONFTREE_H
