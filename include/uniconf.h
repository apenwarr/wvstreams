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

#include "wvhashtable.h"
#include "wvstringlist.h"
#include "uniconfkey.h"

class WvStream;
class WvStringTable;

class UniConf;
class UniConfDict;

extern UniConfDict null_wvhconfdict;


/**
 * A UniConfGen knows how to generate new UniConf objects in its tree.  It
 * may also know how to load/save its tree using some kind of permanent
 * storage (like a disk file, a central HConf server, or whatever).
 */
class UniConfGen
{
public:
    UniConfGen() {}
    virtual ~UniConfGen();
    
    // this function may return NULL if the object "shouldn't" exist
    // (in the opinion of the generator)
    virtual UniConf *make_tree(UniConf *parent, const UniConfKey &key);
   
    virtual void enumerate_subtrees(UniConf *conf, bool recursive);
    virtual void update(UniConf *&h);
    virtual void pre_get(UniConf *&h);
    virtual bool isok() { return true; }

    // Updates all data I am responsible for
    virtual void update_all();
    
    // the default load/save functions don't do anything... you might not
    // need them to.
    virtual void load();
    virtual void save();

};


/**
 * UniConf objects are the root, branches, and leaves of the configuration
 * tree.  Each one has a parent, name=value, and children, all of which are
 * optional (although the name is usually useful).
 * 
 * The nice thing about this is you can write classes that use a UniConf
 * configuration tree, and then instead hand them a subtree if you want.
 */
class UniConf
{
public:
    UniConf *parent;       // the 'parent' of this subtree
    UniConfKey name;       // the name of this entry

private:
    WvString value;        // the contents of this entry
    UniConfDict *children; // list of all child nodes of this node (subkeys)
    UniConfGen *generator; // subtree generator for this tree
public:    
    UniConf *defaults;     // a tree possibly containing default values
    
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
	
	// the 'obsolete' flags can be set true by the generator, if it
	// cares. In that case the generator should also clear them upon
	// request.
	child_obsolete:1,  // some data in the subtree has obsolete=1
	obsolete:1,        // need to re-autogen this data before next use

        // The 'waiting' flags can be set to true by the generator, if it
        // cares. In the case of a waiting flag being set, it means the
        // generator is waiting for data to be returned to it, and the
        // uniconf value is unstable at best, unusable at worst.. so don't
        // use it until the bit is removed.
        child_waiting:1,   // some data in the subtree has waiting=1
        waiting:1;         // need to actually retrieve data before next use.

    UniConf();
    UniConf(UniConf *_parent, const UniConfKey &_name);
    ~UniConf();
    void init();
    
    // fake copy constructor to prevent you from hurting yourself
    UniConf(const UniConf &);
    
    UniConf *top();
    UniConfKey full_key(UniConf *top = NULL) const;
    
    UniConf *gen_top();
    UniConfKey gen_full_key();
    
    bool has_children() const
        { return (children != NULL); }
    // checks generator, then returns children != NULL
    // Needed for iterator over client connection
    bool check_children(bool recursive = false);
    UniConf *find(const UniConfKey &key);
    UniConf *find_make(const UniConfKey &key);
    UniConf &operator[](const UniConfKey &key)
        { return *find_make(key); }

    // Updates me
    void update();
    void remove(const UniConfKey &key = UniConfKey::EMPTY);
    
    UniConf *find_default(const UniConfKey &key = UniConfKey::EMPTY) const;
    
    // exactly the same as find_make() and operator[]... hmm.
    // Unnecessary?
    UniConf &get(const UniConfKey &key)
        { return *find_make(key); }
    
    // another convenience function, suspiciously similar to cfg[key] = v.
    // Also unnecessary?
    void set(const UniConfKey &key, WvStringParm v)
        { get(key).set(v); }
    
    // Reassign the 'value' of this object to something.
    void set_without_notify(WvStringParm s);
    void set(WvStringParm s);
    void mark_notify();
    const UniConf &operator= (WvStringParm s) { set(s); return *this; }
    const UniConf &operator= (const UniConf &s) { set(s); return *this; }
    
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
    void _dump(WvStream &s, bool everything, WvStringTable &keytable);
    void dump(WvStream &s, bool everything = false);

    // Functions to enable the masking of generators
    // Set my generator to be gen, then run load.
    bool hasgen()   { return generator != NULL; }
    bool checkgen() { return hasgen() && generator->isok(); }
    bool comparegen(UniConfGen *gen) { return gen == generator; }
    void mount(UniConfGen *gen);
    void unmount();

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
    
    friend class UniConfGen;
};


DeclareWvDict(UniConf, UniConfKey, name);


#endif // __UNICONF_H
