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

class UniConfTreeBaseDict;

/**
 * UniConfTreeBase the common base implementation for UniConfTree.
 */
class UniConfTreeBase : public UniConfPair
{
protected:
    UniConfTreeBase *xparent; /*!< the parent of this subtree */
    UniConfTreeBaseDict *xchildren; /*!< the children of this node */

    UniConfTreeBase(UniConfTreeBase *parent, const UniConfKey &key,
        WvStringParm value);

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
     * Returns true if the node has children.
     * @return true if the node has children
     */
    bool haschildren() const;

private:
    void link(UniConfTreeBase *node);
    void unlink(UniConfTreeBase *node);
};

DeclareWvDict(UniConfTreeBase, UniConfKey, key());



/**
 * An iterator that walks over all elements on one level of a
 * UniConfTreeBase.
 */
class UniConfTreeBase::Iter
{
    UniConfTreeBase &tree;
    UniConfTreeBaseDict::Iter it;

public:
    Iter(UniConfTreeBase &tree);

    Iter(const Iter &other);

    void rewind();

    inline bool next()
    {
        return it.next();
    }
    inline UniConfTreeBase *ptr() const
    {
        return it.ptr();
    }
    WvIterStuff(UniConfTreeBase);
};



/**
 * UniConfTree serves as a base from which UniConfGen backing
 * store implementations may be constructed.
 *
 * @param T the name of the concrete subclass of UniConfTree
 */
template<class T>
class UniConfTreeGeneric : public UniConfTreeBase
{
    DeclareWvDict3(T, Dict, UniConfKey, key(),);

public:
    /**
     * Creates a node and links it to a subtree.
     * @param parent the parent node, or NULL
     * @param key the key
     * @param value the value
     */
    inline UniConfTreeGeneric(T *parent, const UniConfKey &key,
        WvStringParm value) :
        UniConfTreeBase(parent, key, value)
    {
    }

    /**
     * Unlinks the node from its parent, destroys it along with
     * its contents and children.
     */
    inline ~UniConfTreeGeneric()
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
     * Finds the node for the specified key.
     * @param key the key
     * @return the node, or NULL
     */
    inline T *find(const UniConfKey &key) const
    {
        return static_cast<T*>(UniConfTreeBase::_find(key));
    }
    
    /**
     * Finds the direct child node for the specified key.
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
        Dict *dict = reinterpret_cast<Dict*>(xchildren);
        xchildren = NULL;
        delete dict;
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
 * A plain UniConfTree.
 */
class UniConfTree : public UniConfTreeGeneric<UniConfTree>
{
public:
    inline UniConfTree(UniConfTree *parent,
        const UniConfKey &key,
        WvStringParm value) :
        UniConfTreeGeneric<UniConfTree>(parent, key, value)
    {
    }
};


#endif //__UNICONFTREE_H
