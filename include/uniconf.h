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
DeclareWvCallback(2, void, UniConfCallback, UniConf &, void *);


/**
 * UniConf objects are the root, branches, and leaves of the configuration
 * tree.  Each one has a parent, name=value, and children, all of which are
 * optional (although the name is usually useful).
 * 
 * The nice thing about this is you can write classes that use a UniConf
 * configuration tree, and then instead hand them a subtree if you want.
 */
class UniConf : public UniConfTree<UniConf>
{
    // fake copy constructor to prevent you from hurting yourself
    UniConf(const UniConf &);

    UniConfKey prefix;
    UniConfRoot *uniconfroot;

    UniConf(UniConf *_parent, const UniConfKey &_name);

public:
    typedef UniConfTree<UniConf> Tree;
    UniConf *defaults;     // a tree possibly containing default values
    
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
     * Creates an empty UniConf tree with nothing mounted.
     */
    UniConf();
    
    /**
     * Destroys the tree and possibly discards uncommitted data.
     */
    ~UniConf();
    
    using Tree::parent;
    using Tree::root;
    using Tree::key;
    using Tree::fullkey;

private:
    using Tree::setparent;
    using Tree::find;
    using Tree::findchild;
    
private:
    UniConf *findormake(const UniConfKey &key)
    {
        UniConf *node = this;
        UniConfKey::Iter it(key);
        it.rewind();
        while (it.next())
        {
            UniConf *prev = node;
            node = prev->findchild(it());
            if (! node)
                node = new UniConf(prev, it());
        }
        return node;
    }

public:
    bool haschildren();

    WvString get(const UniConfKey &key);
    
    int getint(const UniConfKey &key)
    {
        return get(key).num();
    }

    WvString value()
    {
        return get(UniConfKey::EMPTY);
    }

    bool exists(const UniConfKey &key)
    {
        return ! get(key).isnull();
    }
    
    bool set(const UniConfKey &key, WvStringParm value);

    bool setint(const UniConfKey &key, int value)
    {
        return set(key, WvString(value));
    }

    void remove(const UniConfKey &key)
    {
        set(key, WvString::null);
    }

    bool zap(const UniConfKey &key = UniConfKey::EMPTY);

    // can't quite remove this yet
    UniConf &operator[] (const UniConfKey &key)
    {
        return *findormake(key);
    }

    WvStringParm operator= (WvStringParm value)
    {
        set(UniConfKey::EMPTY, value);
        return value;
    }

    bool commit(const UniConfKey &key = UniConfKey::EMPTY,
        UniConf::Depth depth = INFINITE);
    
    bool refresh(const UniConfKey &key = UniConfKey::EMPTY,
        UniConf::Depth depth = INFINITE);
    
    /**
     * @internal
     * Prints the entire contents of this subtree to a stream
     * for debugging purposes.
     * @param stream the stream
     * @param everything if true, also prints empty values
     */
    void dump(WvStream &stream, bool everything = false);

    // FIXME: temporary placeholders
    UniConfGen *mount(const UniConfLocation &location);
    void mountgen(UniConfGen *gen);
    void unmount();
    bool ismountpoint();
    bool isok();

    void addwatch(const UniConfKey &key, UniConf::Depth depth,
        const UniConfCallback &cb, void *userdata) { }
    void delwatch(const UniConfKey &key, UniConf::Depth depth,
        const UniConfCallback &cb, void *userdata) { }

    void attach(WvStreamList *streamlist) { }
    void detach(WvStreamList *streamlist) { }

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

public:
    /**
     * Creates an empty UniConf tree with no mounted stores.
     */
    UniConfRoot();

    ~UniConfRoot();

    /**
     * Fetches a string value from the registry.
     * @param key the key
     * @param defvalue the default value, defaults to WvString::null
     * @return the value, or defvalue if the key does not exist
     */
    WvString get(const UniConfKey &key,
        WvStringParm defvalue = WvString::null);
    
    /**
     * Stores a string value into the registry.
     * @param key the key
     * @param value the value, if WvString::null deletes the key
     *        and all of its children
     * @return true on success
     */
    bool set(const UniConfKey &key, WvStringParm value);
    
    /**
     * Removes the children of the specified key.
     * @param key the key
     * @return true on success
     */
    bool zap(const UniConfKey &key);

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
     * Refreshes information about the specified key recursively.
     * <p>
     * May discard uncommitted data.
     * </p>
     * @param key the key
     * @param depth the recursion depth, default is INFINITE
     * @return true on success
     * @see uniconf::depth
     */
    bool refresh(const UniConfKey &key = UniConfKey::EMPTY,
        UniConf::Depth depth = UniConf::INFINITE);

    /**
     * Commits information about the specified key recursively.
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
     
    // FIXME: need a better interface for mount point stuff
    UniConfGen *mount(const UniConfKey &key,
        const UniConfLocation &location, bool refresh);
    void mountgen(const UniConfKey &key, UniConfGen *gen,
        bool refresh);
    void unmount(const UniConfKey &key);
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
    UniConf *xroot;

public:
    Iter(UniConf &root);

    using UniConfRoot::Iter::rewind;
    using UniConfRoot::Iter::next;
    using UniConfRoot::Iter::key;
    
    inline UniConf *root() const
    {
        return xroot;
    }

    UniConf *ptr() const;
    WvIterStuff(UniConf);
};



/**
 * This iterator performs pre-order traversal of a subtree.
 */
class UniConf::RecursiveIter
{
    DeclareWvList3(UniConf::Iter, IterList, )
    UniConf::Iter top;
    UniConf::Depth depth;
    UniConf *current;
    IterList itlist;
    bool first;

public:
    RecursiveIter(UniConf &root,
        UniConf::Depth depth = UniConf::INFINITE);

    void rewind();
    bool next();

    inline UniConf *ptr() const
    {
        return current;
    }
    inline UniConf *root() const
    {
        return top.root();
    }

    WvIterStuff(UniConf);
};


#if 0
/**
 * FIXME: WHAT DOES THIS DO?
 */
class UniConf::XIter
{
public:
    int skiplevel;
    UniConf *top;
    UniConfKey key;
    WvLink _toplink, *toplink;
    UniConfDict::Iter i;
    XIter *subiter;
    int going;
    
    XIter(UniConf &_top, const UniConfKey &_key);
    ~XIter()
        { unsub(); }
    
    void unsub()
        { if (subiter) delete subiter; subiter = NULL; }
    
    void rewind()
        { unsub(); i.rewind(); going = 0; }
    
    WvLink *cur()
        { return subiter ? subiter->cur() : (going==1 ? toplink : NULL); }
    
    WvLink *_next();
    
    WvLink *next()
    {
	WvLink *l;
	while ((l = _next()) != NULL && ptr() == NULL);
	return l;
    }
    
    UniConf *ptr() const
    {
        return key.isempty() ?
            top : (subiter ? subiter->ptr() : NULL);
    }
    
    WvIterStuff(UniConf);
};
#endif


#if 0
// UniConf::Sorter is like UniConf::Iter, but allows you to sort the list.
typedef WvSorter<UniConf, UniConfDict, UniConf::Iter>
    _UniConfSorter;
class UniConf::Sorter : public _UniConfSorter
{
public:
    Sorter(UniConf &h, RealCompareFunc *cmp)
	: _UniConfSorter(h.check_children() ? *h.children : null_wvhconfdict,
			 cmp)
	{ }
};


// UniConf::RecursiveSorter is the recursive version of UniConf::Sorter.
typedef WvSorter<UniConf, UniConfDict, UniConf::RecursiveIter> 
    _UniConfRecursiveSorter;
class UniConf::RecursiveSorter : public _UniConfRecursiveSorter
{
public:
    RecursiveSorter(UniConf &h, RealCompareFunc *cmp)
	: _UniConfRecursiveSorter(h.check_children() ? *h.children : null_wvhconfdict,
				  cmp)
	{ }
};
#endif


#endif // __UNICONF_H
