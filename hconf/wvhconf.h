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

extern WvHConfDict null_wvhconfdict;


/**
 * A class to allow case-insensitive string comparisons.  Without this,
 * WvHConf keys are case-sensitive, which is undesirable.
 */
class WvHConfString : public WvString
{
public:
    WvHConfString(WvStringParm s) : WvString(s)
        { }
    WvHConfString(const char *s) : WvString(s)
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
    WvHConfKey(const WvHConfKey &key, int offset = 0, int max = -1);
    
    WvHConfString printable() const;
    operator WvString () const { return printable(); }
    
    WvHConfKey skip(int offset) const
        { return WvHConfKey(*this, offset); }
    WvHConfKey header(int max) const
        { return WvHConfKey(*this, 0, max); }
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
    
    // this function may return NULL if the object "shouldn't" exist
    // (in the opinion of the generator)
    virtual WvHConf *make_tree(WvHConf *parent, const WvHConfKey &key);
    
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
    WvHConfDict *children; // list of all child nodes of this node (subkeys)
public:    
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
    WvHConfKey gen_full_key();
    
    bool has_children() const
        { return (children != NULL); }
    
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
    void mark_notify();
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
    // if everything=true, also prints objects with null values.
    void dump(WvStream &s, bool everything = false);
    
    class Iter;
    class RecursiveIter;
    class XIter;
    class Sorter;
    class RecursiveSorter;
    
    friend class Iter;
    friend class RecursiveIter;
    friend class XIter;
    friend class Sorter;
    friend class RecursiveSorter;
    
    friend class WvHConfGen;
};


DeclareWvDict(WvHConf, WvHConfString, name);


#endif // __WVHCONF_H
