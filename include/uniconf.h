/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * UniConf is the new, improved, hierarchical version of WvConf.  It stores
 * strings in a hierarchy and can load/save them from "various places."
 */
#ifndef __UNICONF_H
#define __UNICONF_H

#include "uniconflocation.h"
#include "uniconfkey.h"
#include "uniconftree.h"
#include "uniconfiter.h"
#include "wvstream.h"
#include "wvstreamlist.h"
#include "wvstringtable.h"
#include "wvcallback.h"

class WvStream;
class UniConfGen;
class UniConf;
class UniConfRoot;

// parameters are: UniConf object, userdata
DeclareWvCallback(2, void, UniConfCallback, const UniConf &, void *);


/**
 * UniConf instances function as handles to subtrees of a UniConf
 * tree and expose a high-level interface for clients.
 * <p>
 * All operations are marked "const" unless they modify the target
 * of the handle.  In effect, this grants UniConf handles the
 * same semantics as pointers where a const pointer may point
 * to a non-const object, which simply means that the pointer
 * cannot be reassigned.
 * </p><p>
 * When handles are returned from functions, they are always marked
 * const to guard against accidentally assigning to a temporary by
 * an expression such as <code>cfg["foo"] = cfg["bar"]</code>.
 * Instead this must be written as
 * <code>cfg["foo"].set(cfg["bar"].get())</code> which is slightly
 * less elegant but avoids many subtle mistakes.  Also for this
 * reason, unusual cast operators, assignment operators,
 * or copy constructors are not provided.  Please do not add any.
 * </p>
 */
class UniConf
{
    UniConfRoot *xmanager;
    UniConfKey xfullkey;

public:
    /**
     * An enumeration that represents the traversal depth for recursive
     * operations.
     */
    enum Depth
    {
        ZERO = 0, /*!< considers this key only */
        ONE = 1,  /*!< considers this key and all direct children */
        INFINITE = 2, /*!< considers this key and all descendents */
        CHILDREN = 3, /*!< considers all direct children */
        DESCENDENTS = 4 /*!< considers all descendents */
    };

    /**
     * Creates a handle to the specified subtree.
     * @param manager the UniConfRoot that manages the subtree,
     *        may be NULL to signal an invalid handle
     * @param fullkey the path of the subtree
     */
    explicit UniConf(UniConfRoot *manager,
        const UniConfKey &fullkey = UniConfKey::EMPTY) :
        xmanager(manager), xfullkey(fullkey)
    {
    }
    
    /**
     * Copies a UniConf handle.
     * @param other the handle to copy
     */
    UniConf(const UniConf &other) :
        xmanager(other.xmanager), xfullkey(other.xfullkey)
    {
    }
    
    /**
     * Destroys the UniConf handle.
     */
    ~UniConf()
    {
    }

    /**
     * Returns a handle to the root of the tree.
     * @return the handle
     */
    UniConf root() const
    {
        return UniConf(xmanager, UniConfKey::EMPTY);
    }

    /**
     * Returns a handle to the parent of this node.
     * @return the handle
     */
    UniConf parent() const
    {
        return UniConf(xmanager, xfullkey.removelast());
    }

    /**
     * Returns a pointer to the UniConfRoot that manages this node.
     * @return the manager, may be NULL to signal an invalid handle
     */
    inline UniConfRoot *manager() const
    {
        return xmanager;
    }

    /**
     * Returns true if the handle is invalid (NULL).
     * @return true in that case
     */
    inline bool isnull() const
    {
        return xmanager == NULL;
    }

    /**
     * Returns the full path of this node.
     * @return the path
     */
    inline UniConfKey fullkey() const
    {
        return xfullkey;
    }

    /**
     * Returns the path of this node relative to its parent.
     * @return the path
     */
    inline UniConfKey key() const
    {
        return xfullkey.last();
    }


    /**
     * Returns a handle over a subtree below this key.
     * @param key the path of the subtree to be appended to the full
     *        path of this handle to obtain the full path of
     *        the new handle
     * @return the handle
     */
    const UniConf operator[] (const UniConfKey &key) const
    {
        return UniConf(xmanager, UniConfKey(xfullkey, key));
    }

    /**
     * Changes the target of the handle.
     * @param other the new target
     * @return a reference to self
     */
    UniConf &operator= (const UniConf &other)
    {
        xmanager = other.xmanager;
        xfullkey = other.xfullkey;
        return *this;
    }

    /**
     * Returns true if this key exists without fetching its value.
     * <p>
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the value.
     * </p>
     * @return true if the key exists
     */
    bool exists() const;

    /**
     * Returns true if this key has children.
     * <p>
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the keys.
     * </p>
     * @return true if the key has children
     */
    bool haschildren() const;

    /**
     * Fetches the string value for this key from the registry.
     * @param defvalue the default value, defaults to WvString::null
     * @return the value, or defvalue if the key does not exist
     */
    WvString get(WvStringParm defvalue = WvString::null) const;
    
    /**
     * Fetches the integer value for this key from the registry.
     * @param defvalue the default value, defaults to 0
     * @return the value, or defvalue if the key does not exist
     */
    int getint(int defvalue = 0) const;

    /**
     * Stores a string value for this key into the registry.
     * @param value the value, if WvString::null deletes the key
     *        and all of its children
     * @return true on success
     */
    bool set(WvStringParm value) const;

    /**
     * Stores a string value for this key into the registry.
     * @return true on success
     */
    inline bool set(WVSTRING_FORMAT_DECL) const
    {
        return set(WvString(WVSTRING_FORMAT_CALL));
    }

    /**
     * Stores an integer value for this key into the registry.
     * @param value the value
     * @return true on success
     */
    bool setint(int value) const;

    /**
     * Removes this key and all of its children from the registry.
     * @return true on success
     */
    inline bool remove() const
    {
        return set(WvString::null);
    }

    /**
     * Removes the children of this key from the registry.
     * @return true on success
     */
    bool zap() const;

    /**
     * Refreshes information about this key recursively.
     * <p>
     * May discard uncommitted data.
     * </p>
     * @param depth the recursion depth, default is INFINITE
     * @return true on success
     * @see UniConf::Depth
     */
    bool refresh(Depth depth = INFINITE) const;
    
    /**
     * Commits information about this key recursively.
     * @param depth the recursion depth, default is INFINITE
     * @return true on success
     * @see UniConf::Depth
     */
    bool commit(Depth depth = INFINITE) const;

    /**
     * Requests notification when any the keys covered by the
     * recursive depth specification changes.
     * @param depth the recursion depth identifying keys of interest
     * @param callback the callback
     * @param userdata the user data
     */
    void addwatch(UniConf::Depth depth,
        const UniConfCallback &callback, void *userdata) const { }

    /**
     * Cancels a previously registered notification request.
     * @param depth the recursion depth identifying keys of interest
     * @param callback the callback
     * @param userdata the user data
     */
    void delwatch(UniConf::Depth depth,
        const UniConfCallback &callback, void *userdata) const { }
    
    /**
     * Mounts a generator at this key using a moniker.
     * @param location the generator moniker
     * @param refresh if true, automatically refreshes the
     *        generator after mounting, defaults to true
     * @return the generator instance pointer, or NULL on failure
     */
    UniConfGen *mount(const UniConfLocation &location,
        bool refresh = true) const;
    
    /**
     * Mounts a generator at this key.
     * <p>
     * Takes ownership of the supplied generator instance.
     * </p>
     * @param generator the generator instance
     * @param refresh if true, automatically refreshes the
     *        generator after mounting, defaults to true
     */
    void mountgen(UniConfGen *gen, bool refresh = true) const;
    
    /**
     * @internal
     * Prints the entire contents of this subtree to a stream
     * for debugging purposes.
     * @param stream the stream
     * @param everything if true, also prints empty values
     */
    void dump(WvStream &stream, bool everything = false) const;

    // FIXME: temporary placeholders
    void unmount(bool commit = true) const;
    bool ismountpoint() const;
    bool isok() const;

    // FIXME: not final API!
    void setdefaults(const UniConf &subtree) const { }

    class Iter;
    class RecursiveIter;
    
    friend class Iter;
    friend class RecursiveIter;
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
   
    class NodeIter;
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
    UniConfInfoTree root;
    WvStreamList *streamlist;

    /**
     * undefined.
     */
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
     * @see UniConf::Depth
     */
    bool refresh(const UniConfKey &key = UniConfKey::EMPTY,
        UniConf::Depth depth = UniConf::INFINITE);

    /**
     * Commits information about a key recursively.
     * @param key the key
     * @param depth the recursion depth, default is INFINITE
     * @return true on success
     * @see UniConf::Depth
     */
    bool commit(const UniConfKey &key = UniConfKey::EMPTY,
        UniConf::Depth depth = UniConf::INFINITE);

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
     * @param refresh if true, automatically refreshes the
     *        generator after mounting
     * @return the generator instance pointer, or NULL on failure
     */
    UniConfGen *mount(const UniConfKey &key,
        const UniConfLocation &location, bool refresh);
    
    /**
     * Mounts a generator at a key.
     * <p>
     * Takes ownership of the supplied generator instance.
     * </p>
     * @param key the key
     * @param generator the generator instance
     * @param refresh if true, automatically refreshes the
     *        generator after mounting
     */
    void mountgen(const UniConfKey &key, UniConfGen *gen,
        bool refresh);
     
    // FIXME: need a better interface for mount point stuff
    void unmount(const UniConfKey &key, bool commit);
    bool ismountpoint(const UniConfKey &key);
    bool isok(const UniConfKey &key);

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
        UniConf::Depth);
    bool dorecursive(GenFunc func,
        const UniConfKey &key, UniConf::Depth depth);
    bool dorecursivehelper(GenFunc func,
        UniConfInfoTree *node, UniConf::Depth depth);

    static bool genrefreshfunc(UniConfGen *gen,
        const UniConfKey &key, UniConf::Depth depth);
    static bool gencommitfunc(UniConfGen *gen,
        const UniConfKey &key, UniConf::Depth depth);

    void recursiveattach(UniConfInfoTree *node);
    void recursivedetach(UniConfInfoTree *node);
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


/**
 * This iterator walks through all immediate children of a
 * UniConf node.
 */
class UniConf::Iter : private UniConfRoot::Iter
{
    UniConf xroot;
    UniConf current;

public:
    Iter(const UniConf &root);

    using UniConfRoot::Iter::rewind;
    using UniConfRoot::Iter::key;
    
    bool next();

    inline const UniConf *ptr() const
    {
        return & current;
    }
    WvIterStuff(const UniConf);
};



/**
 * This iterator performs pre-order traversal of a subtree.
 */
class UniConf::RecursiveIter
{
    DeclareWvList3(UniConf::Iter, IterList, )
    UniConf xroot;
    UniConf::Iter top;
    UniConf::Depth depth;
    UniConf current;
    IterList itlist;
    bool first;

public:
    RecursiveIter(const UniConf &root,
        UniConf::Depth depth = UniConf::INFINITE);

    void rewind();
    bool next();

    inline const UniConf *ptr() const
    {
        return & current;
    }
    WvIterStuff(const UniConf);
};


#endif // __UNICONF_H
