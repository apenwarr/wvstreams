/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a hierarchical registry abstraction.
 */
#ifndef __UNICONF_H
#define __UNICONF_H

#include "uniconfiter.h"
#include "uniconfgen.h"
#include "uniconfwatch.h"
#include "wvvector.h"

class WvStream;
class UniConfGen;
class UniConf;
class UniConfRoot;

/**
 * UniConf instances function as handles to subtrees of a UniConf
 * tree and expose a high-level interface for clients.
 * 
 * All operations are marked "const" unless they modify the target
 * of the handle.  In effect, this grants UniConf handles the
 * same semantics as pointers where a const pointer may point
 * to a non-const object, which simply means that the pointer
 * cannot be reassigned.
 * 
 * When handles are returned from functions, they are always marked
 * const to guard against accidentally assigning to a temporary by
 * an expression such as <code>cfg["foo"] = cfg["bar"]</code>.
 * Instead this must be written as
 *     cfg["foo"].set(cfg["bar"].get())
 * which is slightly
 * less elegant but avoids many subtle mistakes.  Also for this
 * reason, unusual cast operators, assignment operators,
 * or copy constructors are not provided.  Please do not add any.
 * 
 */
class UniConf
{
    friend class UniConfRoot;
    
    UniConfRoot *xroot;
    UniConfKey xfullkey;

    /**
     * Creates a handle to the specified subtree of the given root.
     * 
     * You can't create non-NULL UniConf objects yourself - ask UniConfRoot
     * or another UniConf object to make one for you.
     */
    UniConf(UniConfRoot *root, const UniConfKey &fullkey = UniConfKey::EMPTY)
        : xroot(root), xfullkey(fullkey)
        { }
    
public:
    /**
     * Creates a NULL UniConf handle, useful for reporting errors.
     */
    UniConf() 
        : xroot(NULL), xfullkey(UniConfKey::EMPTY)
        { }
    
    /**
     * Creates a UniConf handle at the base of the specified root.
     * This is mainly for convenience, since a lot of people will want one
     * of these.
     */
    UniConf(UniConfRoot &root)
        : xroot(&root), xfullkey(UniConfKey::EMPTY)
        { }
    
    /**
     * Copies a UniConf handle.
     * @param other the handle to copy
     */
    UniConf(const UniConf &other)
        : xroot(other.xroot), xfullkey(other.xfullkey)
    {
    }
    
    /** Destroys the UniConf handle. */
    ~UniConf()
    {
    }

    /** Returns a handle to the root of the tree. */
    UniConf root() const
        { return UniConf(xroot, UniConfKey::EMPTY); }

    /** Returns a handle to the parent of this node. */
    UniConf parent() const
        { return UniConf(xroot, xfullkey.removelast()); }
    
    /**
     * Returns a pointer to the UniConfRoot that manages this node.  This
     * may be NULL, to signal an invalid handle.
     */
    UniConfRoot *rootobj() const
        { return xroot; }

    /** Returns true if the handle is invalid (NULL). */
    bool isnull() const
        { return xroot == NULL; }

    /** Returns the full path of this node, starting at the root. */
    UniConfKey fullkey() const
        { return xfullkey; }
    
    /** Returns the full path of this node, starting at the given key.
     * Assumes that k is an ancestor of fullkey(). */
    UniConfKey fullkey(const UniConfKey &k) const;
    
    /** Returns the full path of this node, starting at the given handle. */
    UniConfKey fullkey(const UniConf &cfg) const
        { return fullkey(cfg.fullkey()); }

    /** Returns the path of this node relative to its parent. */
    UniConfKey key() const
        { return xfullkey.last(); }


    /**
     * Returns a handle for a subtree below this key. 'key' is the path
     * of the subtree to be appended to the full path of this handle to
     * obtain the full path of the new handle.
     */
    const UniConf operator[] (const UniConfKey &key) const
        { return UniConf(xroot, UniConfKey(xfullkey, key)); }

    /** Reassigns the target of this handle to match a different one. */
    UniConf &operator= (const UniConf &other)
    {
        xroot = other.xroot;
        xfullkey = other.xfullkey;
        return *this;
    }

    /**
     * Without fetching its value, returns true if this key exists.
     * 
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the value.
     */
    bool exists() const;

    /**
     * Returns true if this key has children.
     * 
     * This is provided because it is often more efficient to
     * test existance than to actually retrieve the keys.
     */
    bool haschildren() const;

    /**
     * Fetches the string value for this key from the registry.  If the
     * key is not found, returns 'defvalue' instead.
     */
    WvString get(WvStringParm defvalue = WvString::null) const;
    
    /**
     * Fetches the integer value for this key from the registry.  If the
     * key is not found, returns 'defvalue' instead.
     */
    int getint(int defvalue = 0) const;

    /**
     * Stores a string value for this key into the registry.  If the value
     * is WvString::null, deletes the key and all of its children.
     * Returns true on success.
     */
    bool set(WvStringParm value) const;

    /**
     * Stores a string value for this key into the registry.
     * Returns true on success.
     */
    bool set(WVSTRING_FORMAT_DECL) const
        { return set(WvString(WVSTRING_FORMAT_CALL)); }

    /**
     * Stores an integer value for this key into the registry.
     * Returns true on success.
     */
    bool setint(int value) const;

    /**
     * Removes this key and all of its children from the registry.
     * Returns true on success.
     */
    bool remove() const
        { return set(WvString::null); }

    /**
     * Removes all children of this key from the registry.
     * Returns true on success.
     */
    bool zap() const;

    /**
     * Refreshes information about this key recursively.
     * May discard uncommitted data.  'depth' is the recursion depth.
     * Returns true on success.
     * 
     * @see UniConfDepth::Type
     */
    bool refresh(UniConfDepth::Type depth =
        UniConfDepth::INFINITE) const;
    
    /**
     * Commits information about this key recursively.
     * 'depth' is the recursion depth.  Returns true on success.
     * 
     * @see UniConfDepth::Type
     */
    bool commit(UniConfDepth::Type depth =
        UniConfDepth::INFINITE) const;

    /**
     * Mounts a generator at this key using a moniker.
     * 
     * If 'refresh' is true, automatically refresh()es the generator
     * after mounting.
     */
    UniConfGen *mount(WvStringParm moniker, bool refresh = true) const;
    
    /**
     * Mounts a generator at this key.
     * 
     * Takes ownership of the supplied generator instance.
     * 
     * If 'refresh' is true, automatically refresh()es the generator
     * after mounting.
     * 
     * Returns a handle to the mounted generator.  Check
     * ! UniConfMount::isnull() to determine success.
     */
    UniConfGen *mountgen(UniConfGen *gen, bool refresh = true) const;
    
    /**
     * Unmounts the generator providing this key and destroys it.
     */
    void unmount(UniConfGen *gen, bool commit) const;
    
    /**
     * Determines if any generators are mounted at this key.
     */
    bool ismountpoint() const;
    
    /**
     * Finds the generator that owns this key.
     * 
     * If the key exists, returns the generator that provides its
     * contents.  Otherwise returns the generator that would be
     * updated if a value were set.
     * 
     * If non-NULL, 'mountpoint' is set to the actual key where the generator
     * is mounted.
     */
    UniConfGen *whichmount(UniConfKey *mountpoint = NULL) const;

    /**
     * Requests notification when any the keys covered by the
     * recursive depth specification change.
     */
    void addwatch(UniConfDepth::Type depth, UniConfWatch *watch) const;

    /**
     * Cancels a previously registered notification request.
     */
    void delwatch(UniConfDepth::Type depth, UniConfWatch *watch) const;

    /**
     * Shortcut for registering a callback-style watch.
     */
    void addwatchcallback(UniConfDepth::Type depth,
        const UniConfCallback &callback, void *userdata) const { }
    
    /**
     * Shortcut for canceling a watch added with addwatchcallback().
     */
    void delwatchcallback(UniConfDepth::Type depth,
        const UniConfCallback &callback, void *userdata) const { }

    /**
     * Shortcut for adding a setbool-style watch.
     */
    void addwatchsetbool(UniConfDepth::Type depth, bool *flag) const { }
    
    /**
     * Shortcut for canceling a watch added with addwatchsetbool().
     */
    void delwatchsetbool(UniConfDepth::Type depth, bool *flag) const { }
    
    /**
     * Prints the entire contents of this subtree to a stream.
     * If 'everything' is true, also prints empty values.
     */
    void dump(WvStream &stream, bool everything = false) const;

    // FIXME: not final API!
    void setdefaults(const UniConf &subtree) const { }

    
    /*** Iterators (see comments in class declaration) ***/

    // internal base class for all of the key iterators
    class IterBase;
    // iterates over direct children
    class Iter;
    // iterates over all descendents in preorder traversal
    class RecursiveIter;
    // iterates over children matching a wildcard
    class XIter;

    // internal base class for sorted key iterators
    class SortedIterBase;
    // sorted variant of Iter
    class SortedIter;
    // sorted variant of RecursiveIter
    class SortedRecursiveIter;
    // sorted variant of XIter
    class SortedXIter;
    
    // lists of iterators
    class IterList;
};


/**
 * @internal
 * An implementation base class for key iterators.
 */
class UniConf::IterBase
{
protected:
    UniConf top;
    UniConf current;

    IterBase(const UniConf &_top)
        : top(_top)
        { }

public:
    const UniConf *ptr() const
        { return &current; }
    WvIterStuff(const UniConf);
};


/*
 * Unfortunately, because of UniConfRoot::BasicIter, if you include uniconf.h
 * you also have to include uniconfroot.h, and vice versa.  Not only that,
 * but since UniConfRoot derives from UniConf, you have to include uniconf.h
 * before defining UniConfRoot.  But since UniConf::Iter needs 
 * UniConfRoot::BasicIter, you have to include uniconfroot.h before defining
 * the following iterators.
 * 
 * So, here's an oddly-placed #include.
 */
#include "uniconfroot.h"


/**
 * This iterator walks through all immediate children of a UniConf node.
 */
class UniConf::Iter : public UniConf::IterBase
{
    UniConfRoot::BasicIter it;
    
public:
    /** Creates an iterator over the direct children of a branch. */
    Iter(const UniConf &_top);

    void rewind()
        { it.rewind(); }
    bool next();
};
DeclareWvList4(UniConf::Iter, IterList, UniConf::IterList, )


/**
 * This iterator performs depth-first traversal of a subtree.
 */
class UniConf::RecursiveIter : public UniConf::IterBase
{
    UniConf::IterList itlist;

public:
    /** Creates a recursive iterator over a branch. */
    RecursiveIter(const UniConf &_top);

    void rewind();
    bool next();
};


/**
 * This iterator walks over all children that match a wildcard
 * pattern.
 * 
 * See UniConfKey::matches(const UniConfKey&) for information about patterns.
 * 
 * Example patterns: (where STAR is the asterisk character, '*')
 *
 * "": a null iterator
 * "a": matches only the key "a" if it exists
 * "STAR": matches all direct children
 * "STAR/foo": matches any existing key "foo" under direct children
 * "STAR/STAR": matches all children of depth exactly 2
 * "foo/...": matches all keys including and below "foo"
 * "foo/STAR/...": matches all keys below "foo"
 * ".../foo/STAR": matches all keys below any subkey named "foo" in the tree
 */
class UniConf::XIter : public UniConf::IterBase
{
    UniConfKey pathead;
    UniConfKey pattail;
    UniConf::XIter *subit;
    UniConf::Iter *it; /*!< iterator over direct children */
    UniConf::RecursiveIter *recit; /*!< iterator over descendents */
    bool ready; /*!< next key is ready */

public:
    /** Creates a wildcard iterator. */
    XIter(const UniConf &_top, const UniConfKey &pattern);
    ~XIter();

    void rewind();
    bool next();
    
private:
    void cleanup();
    bool qnext();
    void enter(const UniConf &child);
};


/**
 * @internal
 * An implementation base class for sorted key iterators.
 * 
 * Unfortunately WvSorter is too strongly tied down to lists and pointers
 * to be of use here.  The main problem is that UniConf::Iter and company
 * return pointers to temporary objects whereas WvSorter assumes that the
 * pointers will remain valid for the lifetime of the iterator.
 */
class UniConf::SortedIterBase : public UniConf::IterBase
{
public:
    typedef int (*Comparator)(const UniConf &a, const UniConf &b);

    /** Default comparator. Sorts alphabetically by full key. */
    static int defcomparator(const UniConf &a, const UniConf &b);

    SortedIterBase(const UniConf &_top, Comparator comparator = defcomparator);
    ~SortedIterBase();

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
    typedef WvVector<UniConf> Vector;
    Vector xkeys;
    
    template<class Iter>
    void populate(Iter &i)
    {
        _purge();
        for (i.rewind(); i.next(); )
            xkeys.append(new UniConf(*i));
        _rewind();
    }
};


/**
 * A sorted variant of UniConf::Iter.
 */
class UniConf::SortedIter : public UniConf::SortedIterBase
{
    UniConf::Iter i;

public:
    SortedIter(const UniConf &_top, Comparator comparator = defcomparator)
        : SortedIterBase(_top, comparator), i(_top)
        { }

    void rewind()
        { populate(i); }
};


/**
 * A sorted variant of UniConf::RecursiveIter.
 */
class UniConf::SortedRecursiveIter : public UniConf::SortedIterBase
{
    UniConf::RecursiveIter i;

public:
    SortedRecursiveIter(const UniConf &_top,
                        Comparator comparator = defcomparator)
        : SortedIterBase(_top, comparator), i(_top)
        { }

    void rewind()
        { populate(i); }
};


/**
 * A sorted variant of UniConf::XIter.
 */
class UniConf::SortedXIter : public UniConf::SortedIterBase
{
    UniConf::XIter i;

public:
    SortedXIter(const UniConf &_top, const UniConfKey &pattern,
                Comparator comparator = defcomparator) 
        : SortedIterBase(_top, comparator), i(_top, pattern) 
        { }

    void rewind()
        { populate(i); }
};


#endif // __UNICONF_H
