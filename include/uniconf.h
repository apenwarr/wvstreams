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
    UniConfGen *generator; // subtree generator for this tree

private:
    // fake copy constructor to prevent you from hurting yourself
    UniConf(const UniConf &);

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
    UniConf(UniConf *_parent, const UniConfKey &_name);
    
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
    UniConf *genroot()
    {
        UniConf *node = this;
        while (! node->generator && node->parent())
            node = node->parent();
        return node;
    }
    UniConfGen *findgen(UniConfKey &genkey)
    {
        UniConf *node = findormake(genkey);
        UniConf *gennode = node->genroot();
        genkey = node->fullkey(gennode);
        return gennode->generator;
    }
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
    bool zap_recursive();

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

    
    /**
     * Commits information about the specified key recursively.
     * <p>
     * The default implementation always returns true.
     * </p>
     * @param key the key
     * @param depth the recursion depth
     * @return true on success
     * @see UniConf::Depth
     */
    bool commit(const UniConfKey &key = UniConfKey::EMPTY,
        UniConf::Depth depth = INFINITE);
    void save()
    {
        commit();
    }
    

    /**
     * refreshes information about the specified key recursively.
     * <p>
     * may discard uncommitted data.
     * </p><p>
     * the default implementation always returns true.
     * </p>
     * @param key the key
     * @param depth the recursion depth
     * @return true on success
     * @see uniconf::depth
     */
    bool refresh(const UniConfKey &key = UniConfKey::EMPTY,
        UniConf::Depth depth = INFINITE);
    void load()
    {
        refresh();
    }
    

    /**
     * @internal
     * Prints the entire contents of this subtree to a stream
     * for debugging purposes.
     * @param stream the stream
     * @param everything if true, also prints empty values
     */
    void dump(WvStream &stream, bool everything = false);

    UniConfGen *mount(const UniConfLocation &location);
    UniConfGen *mountgen(UniConfGen *gen);
    void unmount();

    // temporary placeholders
    bool ismountpoint()
    {
        return generator != NULL;
    }
    bool isok();

    void attach(WvStreamList *streamlist) { }
    void detach(WvStreamList *streamlist) { }

    void addwatch(const UniConfKey &key, UniConf::Depth depth,
        const UniConfCallback &cb, void *userdata) { }
    void delwatch(const UniConfKey &key, UniConf::Depth depth,
        const UniConfCallback &cb, void *userdata) { }

    class Iter;
    class RecursiveIter;
    
    friend class Iter;
    friend class RecursiveIter;
};


/**
 * This iterator walks through all immediate children of a
 * UniConf node.
 */
class UniConf::Iter
{
    UniConf *xroot;
    UniConf::Tree::Iter it;
    UniConfAbstractIter *genit;
    WvStringTable hack; // nasty stuff pending next refactoring phase
    UniConf *current;

public:
    Iter(UniConf &root);
    ~Iter();

    void rewind();

    bool next();

    inline UniConf *ptr() const
    {
        return current;
    }
    inline UniConf *root() const
    {
        return xroot;
    }

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
