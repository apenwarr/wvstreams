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
#include "wvvector.h"

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

    // internal base class for all of the key iterators
    class KeyIterBase;
    // iterates over direct children
    class Iter;
    // iterates over all descendents in preorder traversal
    class RecursiveIter;
    // iterates over children matching a wildcard
    class XIter;
    // internal class for pattern-matching iterators
    class PatternIter;

    // internal base class for sorted key iterators
    class SortedKeyIterBase;
    // sorted variant of Iter
    class SortedIter;
    // sorted variant of RecursiveIter
    class SortedRecursiveIter;
    // sorted variant of XIter
    class SortedXIter;
    
    // iterates over mounts at this location
    class MountIter;

    // lists of iterators
    class IterList;
    class PatternIterList;
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
 * @internal
 * An implementation base class for key iterators.
 */
class UniConf::KeyIterBase
{
protected:
    UniConf xroot;
    UniConf xcurrent;

public:
    KeyIterBase(const UniConf &root) :
        xroot(root), xcurrent(NULL) { }

    inline const UniConf root() const
    {
        return xroot;
    }
    inline const UniConfKey key() const
    {
        return xcurrent.key();
    }
    inline const UniConf *ptr() const
    {
        return & xcurrent;
    }
    WvIterStuff(const UniConf);
};



/**
 * This iterator walks through all immediate children of a
 * UniConf node.
 */
class UniConf::Iter : public UniConf::KeyIterBase
{
    UniConfRoot::Iter it;
    
public:
    /**
     * Creates an iterator over the direct children of a branch.
     * @param root the branch
     */
    Iter(const UniConf &root);

    void rewind();
    bool next();
};
DeclareWvList4(UniConf::Iter, IterList, UniConf::IterList, )


/**
 * This iterator performs pre-order traversal of a subtree.
 */
class UniConf::RecursiveIter : public UniConf::KeyIterBase
{
    UniConf::Iter top;
    UniConfDepth::Type depth;
    UniConf::IterList itlist;
    bool first;

public:
    /**
     * Creates a recursive iterator over a branch.
     * @param root the branch
     * @param depth the recursion depth
     */
    RecursiveIter(const UniConf &root,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    void rewind();
    bool next();
};



/**
 * @internal
 * This iterator walks over all direct children that match a
 * wildcard pattern.  It is used to help construct pattern-matching
 * iterators.
 * <p>
 * Patterns are single segment keys or the special key "*", also
 * known as UniConf::ANY.  It is not currently possible to use
 * wildcards to represent part of a path segment.
 * </p>
 *
 * @see UniConf::XIter
 */
class UniConf::PatternIter : public UniConf::KeyIterBase
{
    UniConfKey xpattern;
    UniConf::Iter *it;
    bool done;
    
public:
    /**
     * Creates a pattern matching iterator.
     * @param root the branch at which to begin iteration
     * @param pattern the pattern for matching children
     */
    PatternIter(const UniConf &root, const UniConfKey &pattern);
    ~PatternIter();

    void rewind();
    bool next();
};
DeclareWvList4(UniConf::PatternIter, PatternIterList, \
    UniConf::PatternIterList, )


/**
 * This iterator walks over all children that match a wildcard
 * pattern.
 * <p>
 * Patterns are simply keys that may have path segments consiting of "*",
 * also known as UniConf::ANY.  It is not currently possible to use
 * wildcards to represent part of a path segment or to indicate optional
 * segments.
 * </p><p>
 * Example patterns: (* has been escaped to disambiguate comment delimiter)
 * <ul>
 * <li>"/": a null iterator</li>
 * <li>"/a": matches only the key "a" if it exists</li>
 * <li>"/&lowast;": matches all direct children</li>
 * <li>"/&lowast;/foo": matches any existing key "foo" under direct children</li>
 * <li>"/&lowast;/&lowast;": matches all children of depth exactly 2</li>
 * </ul>
 * </p>
 */
class UniConf::XIter : public UniConf::KeyIterBase
{
    UniConfKey xpattern;
    UniConf::PatternIterList itlist;

public:
    /**
     * Creates a wildcard iterator.
     * @param root the branch at which to begin iteration
     * @param pattern the pattern for matching children
     */
    XIter(const UniConf &root, const UniConfKey &pattern);

    void rewind();
    bool next();
};


/**
 * @internal
 * An implementation base class for sorted key iterators.
 * 
 * Unfortunately WvSorter is too strongly tied down to lists
 * and pointers to be of use here.  The main problem is that
 * UniConf::Iter and company return pointers to temporary objects
 * whereas WvSorter assumes that the pointers will remain valid
 * for the lifetime of the iterator.
 */
class UniConf::SortedKeyIterBase : public UniConf::KeyIterBase
{
public:
    typedef int (*Comparator)(const UniConf &a, const UniConf &b);

    /**
     * Default comparator.
     * Sorts lexicographically by full key.
     */
    static int defcomparator(const UniConf &a, const UniConf &b);

    SortedKeyIterBase(const UniConf &root,
        Comparator comparator = defcomparator);
    ~SortedKeyIterBase();

    bool next();

private:
    Comparator xcomparator;
    int index;
    int count;
    
    void _purge();
    void _rewind();
    
    static int wrapcomparator(const UniConf **a, const UniConf **b);
    static Comparator innercomparator;

protected:
    typedef WvVector<UniConf*, ShallowBlockOps<UniConf*> > Vector;
    Vector xkeys;
    
    template<class Iter>
    inline void populate(Iter &it)
    {
        _purge();
        for (it.rewind(); it.next(); )
            xkeys.pushback(new UniConf(it()));
        _rewind();
    }
};


/**
 * A sorted variant of UniConf::Iter.
 */
class UniConf::SortedIter : public UniConf::SortedKeyIterBase
{
    UniConf::Iter it;

public:
    SortedIter(const UniConf &root,
        Comparator comparator = defcomparator) :
        SortedKeyIterBase(root, comparator),
        it(root) { }

    void rewind()
    {
        populate(it);
    }
};


/**
 * A sorted variant of UniConf::RecursiveIter.
 */
class UniConf::SortedRecursiveIter : public UniConf::SortedKeyIterBase
{
    UniConf::RecursiveIter it;

public:
    SortedRecursiveIter(const UniConf &root, UniConfDepth::Type depth,
        Comparator comparator = defcomparator) :
        SortedKeyIterBase(root, comparator),
        it(root, depth) { }

    void rewind()
    {
        populate(it);
    }
};


/**
 * A sorted variant of UniConf::XIter.
 */
class UniConf::SortedXIter : public UniConf::SortedKeyIterBase
{
    UniConf::XIter it;

public:
    SortedXIter(const UniConf &root, const UniConfKey &pattern,
        Comparator comparator = defcomparator) :
        SortedKeyIterBase(root, comparator),
        it(root, pattern) { }

    void rewind()
    {
        populate(it);
    }
};


#endif // __UNICONF_H
