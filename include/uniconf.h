/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Defines a hierarchical registry abstraction.
 */
#ifndef __UNICONF_H
#define __UNICONF_H

#include "uniconfdefs.h"
#include "uniconfroot.h"
#include "uniconflocation.h"
#include "uniconfkey.h"
#include "uniconftree.h"
#include "uniconfiter.h"
#include "uniconfgen.h"
#include "wvstringtable.h"

class WvStream;
class UniConfGen;
class UniConf;
class UniConfRoot;
class UniConfMount;

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
     * Reassigns the target of the handle.
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
     * @see UniConfDepth::Type
     */
    bool refresh(UniConfDepth::Type depth =
        UniConfDepth::INFINITE) const;
    
    /**
     * Commits information about this key recursively.
     * @param depth the recursion depth, default is INFINITE
     * @return true on success
     * @see UniConfDepth::Type
     */
    bool commit(UniConfDepth::Type depth =
        UniConfDepth::INFINITE) const;

    /**
     * Mounts a generator at this key using a moniker.
     * @param location the generator moniker
     * @param refresh if true, automatically refreshes the
     *        generator after mounting, defaults to true
     * @return a handle to the mounted generator, check
     *         ! UniConfMount::isnull() to determine success
     */
    UniConfMount mount(const UniConfLocation &location,
        bool refresh = true) const;
    
    /**
     * Mounts a generator at this key.
     * <p>
     * Takes ownership of the supplied generator instance.
     * </p>
     * @param generator the generator instance
     * @param refresh if true, automatically refreshes the
     *        generator after mounting, defaults to true
     * @return a handle to the mounted generator, check
     *         ! UniConfMount::isnull() to determine success
     */
    UniConfMount mountgen(UniConfGen *gen,
        bool refresh = true) const;
    
    /**
     * Determines if any generators are mounted at this key.
     * <p>
     * This is a convenience function.
     * </p>
     * @return true if at least one generator is mounted here
     * @see UniConf::MountIter
     */
    bool ismountpoint() const;

    /**
     * Finds the generator that owns this key.
     * <p>
     * If the key exists, returns the generator that provides its
     * contents.  Otherwise returns the generator that would be
     * updated if a value were set.
     * </p>
     * @return the handle, or a null handle if none
     */
    UniConfMount whichmount() const;

    /**
     * Requests notification when any the keys covered by the
     * recursive depth specification changes.
     * @param depth the recursion depth identifying keys of interest
     * @param callback the callback
     * @param userdata the user data
     */
    void addwatch(UniConfDepth::Type depth,
        const UniConfCallback &callback, void *userdata) const { }

    /**
     * Cancels a previously registered notification request.
     * @param depth the recursion depth identifying keys of interest
     * @param callback the callback
     * @param userdata the user data
     */
    void delwatch(UniConfDepth::Type depth,
        const UniConfCallback &callback, void *userdata) const { }
    
    /**
     * @internal
     * Prints the entire contents of this subtree to a stream
     * for debugging purposes.
     * @param stream the stream
     * @param everything if true, also prints empty values
     */
    void dump(WvStream &stream, bool everything = false) const;

    // FIXME: not final API!
    void setdefaults(const UniConf &subtree) const { }

    /*** Iterators (see comments in class declaration) ***/

    // iterates over direct children
    class Iter;
    // iterates over all descendents in preorder traversal
    class RecursiveIter;
    // iterates over mounts at this location
    class MountIter;
};


/**
 * UniConfMount functions as a handle for a mounted generator.
 * <p>
 * This is safer and more flexible than would be directly exposing
 * generator pointers as the primary interface.  Once a generator
 * has been unmounted, all UniConfMount objects that refer to it
 * become invalid.
 * </p>
 */
class UniConfMount
{
    UniConf xmountpoint;
    UniConfGen *xgen;

public:
    /**
     * Creates an unassigned handle.
     */
    UniConfMount() :
        xmountpoint(NULL), xgen(NULL)
    {
    }

    /**
     * @internal
     * Constructs a UniConfMount handle.
     * @param xmountpoint the mount point
     * @param xgen the generator mounted there
     */
    UniConfMount(const UniConf &mountpoint, UniConfGen *gen) :
        xmountpoint(mountpoint), xgen(gen)
    {
    }

    /**
     * Copies a UniConfMount handle.
     * @param other the handle to copy
     */
    UniConfMount(const UniConfMount &other) :
        xmountpoint(other.xmountpoint), xgen(other.xgen)
    {
    }

    /**
     * Destroys the UniConf handle.
     */
    ~UniConfMount()
    {
    }

    /**
     * Returns a handle to the generator mountpoint.
     * @return the handle
     */
    inline UniConf mountpoint() const
    {
        return xmountpoint;
    }

    /**
     * Returns a pointer to the generator.
     * @return the generator, may be NULL to signal an invalid handle
     */
    inline UniConfGen *gen() const
    {
        return xgen;
    }
    
    /**
     * Returns true if the handle is invalid (NULL).
     * @return true in that case
     */
    inline bool isnull() const
    {
        return xgen == NULL;
    }

    /**
     * Reassigns the target of the handle.
     * @param other the new target
     * @return a reference to self
     */
    UniConfMount &operator= (const UniConfMount &other)
    {
        xmountpoint = other.xmountpoint;
        xgen = other.xgen;
        return *this;
    }
    
    /**
     * Returns true if the generator is ok.
     * Synonym for !isnull() && gen()->isok().
     * @return true in that case
     */
    bool isok() const;

    /**
     * Unmounts the generator and destroys it.
     * Does nothing if the handle is invalid, otherwise destroys
     * the generator and causes the handle to become invalid.
     *
     * @param commit if true, automatically commits the generator
     *        before unmounting, defaults to true
     */
    void unmount(bool commit = true);
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
    UniConfDepth::Type depth;
    UniConf current;
    IterList itlist;
    bool first;

public:
    RecursiveIter(const UniConf &root,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    void rewind();
    bool next();

    inline const UniConf *ptr() const
    {
        return & current;
    }
    WvIterStuff(const UniConf);
};


#endif // __UNICONF_H
