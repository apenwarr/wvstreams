/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvHConf is the new, improved, hierarchical version of WvConf.  It stores
 * strings in a hierarchy and can load/save them from "various places."
 */
#ifndef __WVHCONF_H
#define __WVHCONF_H

#include "wvhashtable.h"
#include "wvstringlist.h"

class WvStream;
class WvHConf;
class WvHConfDict;


/**
 * A class to allow case-insensitive string comparisons.  Without this,
 * WvHConf keys are case-sensitive, which is undesirable.
 */
class WvHConfString : public WvString
{
public:
    WvHConfString(WvStringParm s) : WvString(s)
        { }
    
    bool operator== (WvStringParm s2) const;
};


/**
 * a WvHConfKey is a convenient structure that uniquely identifies a point
 * in the HConf tree and can be converted to/from a WvString.
 */
class WvHConfKey : public WvStringList
{
public:
    WvHConfKey();
    WvHConfKey(const char *key);
    WvHConfKey(WvStringParm key);
    WvHConfKey(WvStringParm section, WvStringParm entry);
    WvHConfKey(const WvHConfKey &key, int offset = 0);
    
    WvString printable() const;
    operator WvString () const { return printable(); }
    
    WvHConfKey skip(int offset) const
        { return WvHConfKey(*this, offset); }
};


/**
 * A WvHConfGen knows how to generate new WvHConf objects in its tree.  It
 * may also know how to load/save its tree using some kind of permanent
 * storage (like a disk file, a central HConf server, or whatever).
 */
class WvHConfGen
{
public:
    WvHConfGen() {}
    virtual ~WvHConfGen();
    
    // both of these functions may return NULL if the object "shouldn't"
    // exist.
    virtual WvHConf *make_tree(WvHConf *parent, const WvHConfKey &key);
    virtual WvHConf *make_obj(WvHConf *parent, WvStringParm name);
    
    virtual void update(WvHConf *h);
    
    // the default load/save functions don't do anything... you might not
    // need them to.
    virtual void load();
    virtual void save();
};


/**
 * WvHConf objects are the root, branches, and leaves of the configuration
 * tree.  Each one has a parent, name=value, and children, all of which are
 * optional (although the name is usually useful).
 * 
 * The nice thing about this is you can write classes that use a WvHConf
 * configuration tree, and then instead hand them a subtree if you want.
 */
class WvHConf
{
public:
    WvHConf *parent;       // the 'parent' of this subtree
    WvHConfString name;    // the name of this entry
private:
    WvString value;        // the contents of this entry
public:    
    WvHConfDict *children; // list of all child nodes of this node (subkeys)
    WvHConf *defaults;     // a tree possibly containing default values
    WvHConfGen *generator; // subtree generator for this tree
    
public:
    bool 
	// the 'dirty' flags are set true by set() and can be cleared by
	// the tree's generator object, if it cares.
	child_dirty:1,     // some data in the subtree has dirty=1
	dirty:1,	   // this data is unsaved
	
	// the 'notify' flags are set true by set() and can be cleared by
	// an external notification system, if there is one.
	child_notify:1,    // some data in the subtree has notify=1
	notify:1,	   // this data changed - notify interested parties
	
	// the 'obsolete' flags can be set true by the generator, if it cares.
	// In that case the generator should also clear them upon request.
	child_obsolete:1,  // some data in the subtree has obsolete=1
	obsolete:1;        // need to re-autogen this data before next use

    WvHConf();
    WvHConf(WvHConf *_parent, WvStringParm _name);
    ~WvHConf();
    void init();
    
    WvHConf *top();
    WvHConfKey full_key(WvHConf *top = NULL) const;
    
    WvHConf *gen_top();
    WvHConfKey gen_full_key() const;
    
    WvHConfDict *needs_children();
    WvHConf *find(const WvHConfKey &key);
    WvHConf *find_make(const WvHConfKey &key);
    WvHConf &operator[](const WvHConfKey &key) { return *find_make(key); }
    
    WvHConf *find_default(WvHConfKey *_k = NULL) const;
    
    // exactly the same as find_make() and operator[]... hmm.
    // Unnecessary?
    WvHConf &get(const WvHConfKey &key)
        { return *find_make(key); }
    
    // another convenience function, suspiciously similar to cfg[key] = v.
    // Also unnecessary?
    void set(const WvHConfKey &key, WvStringParm v)
        { get(key).set(v); }
    
    // Reassign the 'value' of this object to something.
    void set_without_notify(WvStringParm s);
    void set(WvStringParm s);
    void do_notify();
    const WvHConf &operator= (WvStringParm s) { set(s); return *this; }
    const WvHConf &operator= (const WvHConf &s) { set(s); return *this; }
    
    // retrieve the value.  Normally you don't need to call printable()
    // explicitly, since the WvString cast operator does it for you.
    const WvString &printable() const;
    operator const WvString& () const { return printable(); }
    bool operator! () const { return !printable(); }
    const char *cstr() const { return printable().cstr(); }
    int num() const { return printable().num(); }
    
    // load/save the entire tree (including subtrees).
    // Only save if the data is marked 'dirty'.
    void load();
    void save();
    
    // a handy function to print a copy of this subtree to a stream.
    void dump(WvStream &s);
    
    class Iter;
    class RecursiveIter;
    class Sorter;
    class RecursiveSorter;
};


DeclareWvDict(WvHConf, WvHConfString, name);


// this iterator walks through all the immediate children of a
// WvHConf node.
class WvHConf::Iter : public WvHConfDict::Iter
{
public:
    Iter(WvHConf &h)
	: WvHConfDict::Iter(*h.needs_children())
	{ }
    Iter(WvHConfDict &children)
	: WvHConfDict::Iter(children)
	{ }
    
    // we want to skip empty-valued elements in the list, even if
    // they exist.
    WvLink *next()
    {
	WvLink *l;
	while ((l = WvHConfDict::Iter::next()) != NULL && !*ptr())
	    ;
	return l;
    }
};


// this iterator recursively walks through _all_ children, direct and indirect,
// of this node.
class WvHConf::RecursiveIter
{
public:
    WvHConfDict::Iter i;
    RecursiveIter *subiter;
    
    RecursiveIter(WvHConf &h)
	: i(*h.needs_children())
	{ subiter = NULL; }
    RecursiveIter(WvHConfDict &children)
	: i(children)
	{ subiter = NULL; }
    ~RecursiveIter()
        { unsub(); }
    
    void unsub()
        { if (subiter) delete subiter; subiter = NULL; }
    
    void rewind()
        { unsub(); i.rewind(); }
    
    WvLink *cur()
        { return subiter ? subiter->cur() : i.cur(); }
    
    // return the next element, either from subiter or, if subiter is done,
    // the next immediate child of our own.
    WvLink *_next()
    { 
	if (!subiter && i.ptr() && i->children)
	{
	    subiter = new RecursiveIter(*i);
	    subiter->rewind();
	}
	
	if (subiter)
	{
	    WvLink *l = subiter->next();
	    if (l) return l;
	    unsub();
	}
	
	return i.next();
    }
    
    // like _next(), but skip elements with empty values.
    WvLink *next()
    { 
	WvLink *l;
	while ((l = _next()) != NULL && !*ptr())
	    ;
	return l;
    }
    
    WvHConf *ptr() const
        { return subiter ? subiter->ptr() : i.ptr(); }
    
    WvIterStuff(WvHConf);
};


// WvHConf::Sorter is like WvHConf::Iter, but allows you to sort the list.
typedef WvSorter<WvHConf, WvHConfDict, WvHConf::Iter>
    _WvHConfSorter;
class WvHConf::Sorter : public _WvHConfSorter
{
public:
    Sorter(WvHConf &h, RealCompareFunc *cmp)
	: _WvHConfSorter(*h.needs_children(), cmp)
	{ }
};


// WvHConf::RecursiveSorter is the recursive version of WvHConf::Sorter.
typedef WvSorter<WvHConf, WvHConfDict, WvHConf::RecursiveIter> 
    _WvHConfRecursiveSorter;
class WvHConf::RecursiveSorter : public _WvHConfRecursiveSorter
{
public:
    RecursiveSorter(WvHConf &h, RealCompareFunc *cmp)
	: _WvHConfRecursiveSorter(*h.needs_children(), cmp)
	{ }
};


#endif // __WVHCONF_H
