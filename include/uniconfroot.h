/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Defines the root management class for UniConf.
 */
#ifndef __UNICONFROOT_H
#define __UNICONFROOT_H

#include "uniconfdefs.h"
#include "uniconflocation.h"
#include "uniconfkey.h"
#include "uniconftree.h"
#include "wvcallback.h"
#include "wvstringtable.h"

class UniConfGen;
class UniConfInfoTree;
class WvStreamList;

/**
 * Represents the root of a hierarhical registry consisting of pairs
 * of UniConfKeys and associated string values.
 * <p>
 * Any number of data containers may be mounted into the tree at any
 * number of mount points to provide a backing store from which
 * registry keys and values are fetched and into which they are
 * stored.
 * </p>
 * FIXME: only one mount per subtree for the moment
 */
class UniConfRoot
{
    UniConfInfoTree *root;
    WvStreamList *streamlist;

    /** undefined. */
    UniConfRoot(const UniConfRoot &other);

public:
    /**
     * Creates an empty UniConf tree with no mounted stores.
     */
    UniConfRoot();

    /**
     * Destroys the UniConf tree along with all uncommitted data.
     */
    ~UniConfRoot();

    /**
     * Returns true if a key exists without fetching its value.
     * <p>
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the value.
     * </p>
     * @param key the key
     * @return true if the key exists
     */
    bool exists(const UniConfKey &key);
    
    /**
     * Returns true if a key has children.
     * <p>
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the keys.
     * </p>
     * @param key the key
     * @return true if the key has children
     */
    bool haschildren(const UniConfKey &key);

    /**
     * Fetches a string value for a key from the registry.
     * @param key the key
     * @return the value, or WvString::null if the key does not exist
     */
    WvString get(const UniConfKey &key);
    
    /**
     * Stores a string value for a key into the registry.
     * @param key the key
     * @param value the value, if WvString::null deletes the key
     *        and all of its children
     * @return true on success
     */
    bool set(const UniConfKey &key, WvStringParm value);
    
    /**
     * Removes the children of a key from the registry.
     * @param key the key
     * @return true on success
     */
    bool zap(const UniConfKey &key);

    /**
     * Refreshes information about a key recursively.
     * <p>
     * May discard uncommitted data.
     * </p>
     * @param key the key
     * @param depth the recursion depth, default is INFINITE
     * @return true on success
     * @see UniConfDepth::Type
     */
    bool refresh(const UniConfKey &key = UniConfKey::EMPTY,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    /**
     * Commits information about a key recursively.
     * @param key the key
     * @param depth the recursion depth, default is INFINITE
     * @return true on success
     * @see UniConfDepth::Type
     */
    bool commit(const UniConfKey &key = UniConfKey::EMPTY,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    /**
     * Gives UniConf an opportunity to append streams to a streamlist.
     * <p>
     * This method must be called at most once until the next
     * detach().
     * </p>
     * @param streamlist the stream list, non-NULL
     * @see detach(WvStreamList*)
     */
    void attach(WvStreamList *streamlist);

    /**
     * Gives UniConf an opportunity to unlink streams from a streamlist.
     * <p>
     * This method must be called exactly once for each attach().
     * </p>
     * @param streamlist the stream list, non-NULL
     * @see attach(WvStreamList*)
     */
    void detach(WvStreamList *streamlist);
    
    /**
     * Mounts a generator at a key using a moniker.
     * @param key the key
     * @param location the generator moniker
     * @return the generator instance pointer, or NULL on failure
     */
    UniConfGen *mount(const UniConfKey &key,
        const UniConfLocation &location);
    
    /**
     * Mounts a generator at a key.
     * <p>
     * Takes ownership of the supplied generator instance.
     * </p>
     * @param key the key
     * @param generator the generator instance
     * @return the generator instance pointer, or NULL on failure
     */
    UniConfGen *mountgen(const UniConfKey &key, UniConfGen *gen);

    /**
     * Unmounts the generator at a key and destroys it.
     * @param key the key
     * @param generator the generator instance
     */
    void unmount(const UniConfKey &key, UniConfGen *gen);
    
    /**
     * Finds the generator that owns a key.
     * <p>
     * If the key exists, returns the generator that provides its
     * contents.  Otherwise returns the generator that would be
     * updated if a value were set.
     * </p>
     * @param key the key
     * @param mountpoint if not NULL, replaced with the mountpoint
     *        path on success
     * @return the handle, or a null handle if none
     */
    UniConfGen *whichmount(const UniConfKey &key,
        UniConfKey *mountpoint);

    class Iter;
    friend class Iter;

private:
    /**
     * Prunes a branch of the tree beginning at the specified node
     * and moving towards the root.
     * @param node the node
     */
    void prune(UniConfInfoTree *node);

    typedef bool (*GenFunc)(UniConfGen*, const UniConfKey&,
        UniConfDepth::Type);
    bool dorecursive(GenFunc func,
        const UniConfKey &key, UniConfDepth::Type depth);
    bool dorecursivehelper(GenFunc func,
        UniConfInfoTree *node, UniConfDepth::Type depth);

    static bool genrefreshfunc(UniConfGen *gen,
        const UniConfKey &key, UniConfDepth::Type depth);
    static bool gencommitfunc(UniConfGen *gen,
        const UniConfKey &key, UniConfDepth::Type depth);

    void recursiveattach(UniConfInfoTree *node);
    void recursivedetach(UniConfInfoTree *node);
};


/**
 * @internal
 * Used by UniConfRoot to maintain information about mounted
 * subtrees.
 */
class UniConfInfoTree : public UniConfTree<UniConfInfoTree>
{
public:
    UniConfGen *generator;

    UniConfInfoTree(UniConfInfoTree *parent, const UniConfKey &key);
    ~UniConfInfoTree();

    /**
     * Returns true if the node should not be pruned.
     */
    bool isessential()
    {
        return haschildren() || generator;
    }

    /**
     * Returns the nearest node in the info tree to the key.
     * @param key the key
     * @param split set to the number of leading segments used
     * @return the node
     */
    UniConfInfoTree *findnearest(const UniConfKey &key,
        int &split);

    /**
     * Finds or makes an info node for the specified key.
     */
    UniConfInfoTree *findormake(const UniConfKey &key);
   
    // an iterator over nodes that have information about a key
    class NodeIter;
    // an iterator over generators about a key
    class GenIter;
};

/**
 * @internal
 * An iterator over the info nodes that might know something
 * about a key, starting with the nearest match.
 */
class UniConfInfoTree::NodeIter
{
    int bestsplit;
    UniConfInfoTree *bestnode;

    int xsplit;
    UniConfInfoTree *xnode;
    UniConfKey xkey;

public:
    NodeIter(UniConfInfoTree &root, const UniConfKey &key);
    
    void rewind();
    bool next();
    
    inline UniConfKey key() const
    {
        return xkey;
    }
    inline int split() const
    {
        return xsplit;
    }
    inline UniConfKey head() const
    {
        return xkey.first(xsplit);
    }
    inline UniConfKey tail() const
    {
        return xkey.removefirst(xsplit);
    }
    inline UniConfInfoTree *node() const
    {
        return xnode;
    }
    inline UniConfInfoTree *ptr() const
    {
        return node();
    }
    WvIterStuff(UniConfInfoTree);
};


/**
 * @internal
 * An iterator over the generators that might provide a key
 * starting with the nearest match.
 */
class UniConfInfoTree::GenIter : private UniConfInfoTree::NodeIter
{
public:
    GenIter(UniConfInfoTree &root, const UniConfKey &key);

    using UniConfInfoTree::NodeIter::key;
    using UniConfInfoTree::NodeIter::split;
    using UniConfInfoTree::NodeIter::head;
    using UniConfInfoTree::NodeIter::tail;
    using UniConfInfoTree::NodeIter::node;
    
    void rewind();
    bool next();

    inline UniConfGen *ptr() const
    {
        return node()->generator;
    }
    WvIterStuff(UniConfGen);
};

/**
 * This iterator walks through all immediate children of a
 * UniConf subtree.
 */
class UniConfRoot::Iter
{
    UniConfRoot *xroot;
    UniConfKey xkey;

    UniConfInfoTree::GenIter genit;
    WvStringTable hack; // FIXME: ugly hack
    WvStringTable::Iter hackit;

public:
    Iter(UniConfRoot &root, const UniConfKey &key);

    void rewind();
    bool next();
    UniConfKey key() const;
};


#endif //__UNICONFROOT_H
