/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines the root management class for UniConf.
 */

// trust me, this has to come first.  See the weird #include note in
// uniconf.h.
#include "uniconf.h"

#ifndef __UNICONFROOT_H
#define __UNICONFROOT_H

#include "uniconftree.h"
#include "wvcallback.h"
#include "wvstringtable.h"

class UniConfGen;
class UniConfWatch;
class WvStreamList;
class UniConfMountTree;

/**
 * Represents the root of a hierarhical registry consisting of pairs
 * of UniConfKeys and associated string values.
 * 
 * Any number of data containers may be mounted into the tree at any
 * number of mount points to provide a backing store from which
 * registry keys and values are fetched and into which they are
 * stored.
 * 
 * FIXME: only one mount per subtree for the moment
 */
class UniConfRoot : public UniConf
{
    friend class UniConf;
    friend class UniConfGen;
    friend class UniConf::Iter;
    
    UniConfMountTree *mounts;

    /** undefined. */
    UniConfRoot(const UniConfRoot &other);

public:
    /** Creates an empty UniConf tree with no mounted stores. */
    UniConfRoot();
    
    /** 
     * Creates a new UniConf tree and mounts the given moniker at the root.
     * Since most people only want to mount one generator, this should save
     * a line of code here and there.
     */
    UniConfRoot(WvStringParm moniker, bool refresh = true);

    /** 
     * Creates a new UniConf tree and mounts the given generator at the root.
     * Since most people only want to mount one generator, this should save
     * a line of code here and there.
     */
    UniConfRoot(UniConfGen *gen, bool refresh = true);

    /** Destroys the UniConf tree along with all uncommitted data. */
    ~UniConfRoot();
    
    /** Returns a UniConf object that points into this tree somewhere. */

    /* the following bits used to be public, but it causes API confusion
     * because there are several functions with the same names but different
     * parameters from the UniConf object.
     * 
     * Use a UniConf instance instead to do this sort of thing.
     */
protected:
    
    /**
     * Returns true if a key exists without fetching its value.
     * 
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the value.
     * 
     * @param key the key
     * @return true if the key exists
     */
    bool _exists(const UniConfKey &key);
    
    /**
     * Returns true if a key has children.
     * 
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the keys.
     * 
     * @param key the key
     * @return true if the key has children
     */
    bool _haschildren(const UniConfKey &key);

    /**
     * Fetches a string value for a key from the registry.
     * @param key the key
     * @return the value, or WvString::null if the key does not exist
     */
    WvString _get(const UniConfKey &key);
    
    /**
     * Stores a string value for a key into the registry.
     * @param key the key
     * @param value the value, if WvString::null deletes the key
     *        and all of its children
     * @return true on success
     */
    bool _set(const UniConfKey &key, WvStringParm value);
    
    /**
     * Removes the children of a key from the registry.
     * @param key the key
     * @return true on success
     */
    bool _zap(const UniConfKey &key);

    /**
     * Refreshes information about a key recursively.
     * 
     * May discard uncommitted data.
     * 
     * @param key the key
     * @param depth the recursion depth, default is INFINITE
     * @return true on success
     * @see UniConfDepth::Type
     */
    bool _refresh(const UniConfKey &key = UniConfKey::EMPTY,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    /**
     * Commits information about a key recursively.
     * @param key the key
     * @param depth the recursion depth, default is INFINITE
     * @return true on success
     * @see UniConfDepth::Type
     */
    bool _commit(const UniConfKey &key = UniConfKey::EMPTY,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    /**
     * Mounts a generator at a key using a moniker.
     * 
     * Returns the generator instance pointer, or NULL on failure.
     */
    UniConfGen *_mount(const UniConfKey &key, WvStringParm moniker,
        bool refresh);
    
    /**
     * Mounts a generator at a key.
     * Takes ownership of the supplied generator instance.
     * 
     * @param key the key
     * @param gen the generator instance
     * @param refresh if true, refreshes the generator after mount
     * @return the generator instance pointer, or NULL on failure
     */
    UniConfGen *_mountgen(const UniConfKey &key, UniConfGen *gen,
        bool refresh);

    /**
     * Unmounts the generator at a key and destroys it.
     *
     * @param key the key
     * @param gen the generator instance
     * @param commit if true, commits the generator before unmount
     */
    void _unmount(const UniConfKey &key, UniConfGen *gen, bool commit);
    
    /**
     * Finds the generator that owns a key.
     * 
     * If the key exists, returns the generator that provides its
     * contents.  Otherwise returns the generator that would be
     * updated if a value were set.
     * 
     * @param key the key
     * @param mountpoint if not NULL, replaced with the mountpoint
     *        path on success
     * @return the handle, or a null handle if none
     */
    UniConfGen *_whichmount(const UniConfKey &key, UniConfKey *mountpoint);

    /**
     * Determines if a key is a mountpoint.
     */
    bool _ismountpoint(const UniConfKey &key);
    
    /**
     * Requests notification when any the keys covered by the
     * recursive depth specification changes.
     * @param depth the recursion depth identifying keys of interest
     * @param watch the observer to notify
     */
    void _addwatch(const UniConfKey &key, UniConfDepth::Type depth,
        UniConfWatch *watch);

    /**
     * Cancels a previously registered notification request.
     * @param depth the recursion depth identifying keys of interest
     * @param watch the observer to notify
     */
    void _delwatch(const UniConfKey &key, UniConfDepth::Type depth,
        UniConfWatch *watch);

    class BasicIter;
    friend class BasicIter;

private:
    /**
     * Causes an explicit change notification.
     * @param key the key that was changed
     * @param depth the depth of the change
     */
    void delta(const UniConfKey &key, UniConfDepth::Type depth);

    /**
     * Prunes a branch of the tree beginning at the specified node
     * and moving towards the root.
     * @param node the node
     */
    void prune(UniConfMountTree *node);

    typedef bool (*GenFunc)(UniConfGen*, const UniConfKey&,
        UniConfDepth::Type);
    bool dorecursive(GenFunc func,
        const UniConfKey &key, UniConfDepth::Type depth);
    bool dorecursivehelper(GenFunc func,
        UniConfMountTree *node, UniConfDepth::Type depth);

    static bool genrefreshfunc(UniConfGen *gen,
        const UniConfKey &key, UniConfDepth::Type depth);
    static bool gencommitfunc(UniConfGen *gen,
        const UniConfKey &key, UniConfDepth::Type depth);

    /**
     * Called by generators when a key changes.
     */
    void gencallback(const UniConfGen &gen, const UniConfKey &key,
        UniConfDepth::Type depth, void *userdata);
};


/**
 * @internal
 * Used by UniConfRoot to maintain information about mounted
 * subtrees.
 */
class UniConfMountTree : public UniConfTree<UniConfMountTree>
{
public:
    UniConfGenList generators;

    UniConfMountTree(UniConfMountTree *parent, const UniConfKey &key);
    ~UniConfMountTree();

    /** Returns true if the node should not be pruned. */
    bool isessential()
        { return haschildren() || ! generators.isempty(); }

    /**
     * Returns the nearest node in the info tree to the key.
     * @param key the key
     * @param split set to the number of leading segments used
     * @return the node
     */
    UniConfMountTree *findnearest(const UniConfKey &key, int &split);

    /** Finds or makes an info node for the specified key. */
    UniConfMountTree *findormake(const UniConfKey &key);
   
    // an iterator over nodes that have information about a key
    class MountIter;
    // an iterator over generators about a key
    class GenIter;
};


/**
 * @internal
 * An iterator over the UniConfMountTree nodes that might know something
 * about the provided 'key', starting with the nearest match and then
 * moving up the tree.
 */
class UniConfMountTree::MountIter
{
    int bestsplit;
    UniConfMountTree *bestnode;

    int xsplit;
    UniConfMountTree *xnode;
    UniConfKey xkey;

public:
    MountIter(UniConfMountTree &root, const UniConfKey &key);
    
    void rewind();
    bool next();
    
    int split() const
        { return xsplit; }
    UniConfKey key() const
        { return xkey; }
    UniConfKey head() const
        { return xkey.first(xsplit); }
    UniConfKey tail() const
        { return xkey.removefirst(xsplit); }
    UniConfMountTree *node() const
        { return xnode; }
    UniConfMountTree *ptr() const
        { return node(); }
    WvIterStuff(UniConfMountTree);
};


/**
 * @internal
 * An iterator over the generators that might provide a key
 * starting with the nearest match.
 * 
 * eg. if you have something mounted on /foo and /foo/bar/baz, and you ask
 * for a GenIter starting at /foo/bar/baz/boo/snoot, GenIter will give you
 * /foo/bar/baz followed by /foo; MountIter will give you /foo/bar/baz,
 * then /foo/bar, then /foo.
 */
class UniConfMountTree::GenIter : private UniConfMountTree::MountIter
{
    UniConfGenList::Iter *genit; /*!< active generator iterator */

public:
    GenIter(UniConfMountTree &root, const UniConfKey &key);
    ~GenIter();

    using UniConfMountTree::MountIter::split;
    using UniConfMountTree::MountIter::key;
    using UniConfMountTree::MountIter::head;
    using UniConfMountTree::MountIter::tail;
    using UniConfMountTree::MountIter::node;
    
    void rewind();
    bool next();

    UniConfGen *ptr() const
        { return genit ? genit->ptr() : NULL; }
    WvIterStuff(UniConfGen);
};


/**
 * This iterator walks through all immediate children of a
 * UniConf subtree.
 * 
 * This is mainly needed by the (much more useful) UniConf::Iter,
 * RecursiveIter, XIter, etc.
 */
class UniConfRoot::BasicIter
{
    UniConfRoot *xroot;
    UniConfKey xkey;

    UniConfMountTree::GenIter genit;
    WvStringTable hack; // FIXME: ugly hack
    WvStringTable::Iter hackit;

public:
    BasicIter(UniConfRoot &root, const UniConfKey &key);

    void rewind();
    bool next();
    
    UniConfKey key() const;
};


#endif //__UNICONFROOT_H
