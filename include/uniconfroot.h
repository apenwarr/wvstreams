/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines the root management class for UniConf.
 */
#ifndef __UNICONFROOT_H
#define __UNICONFROOT_H

#include "uniconf.h"
#include "uniconftree.h"
#include "unimountgen.h"

/**
 * @internal
 * Holds information about a single watch.
 */
class UniWatch
{
    bool recurse;
    UniConfCallback cb;
    void *cbdata;

public:
    UniWatch(bool _recurse, UniConfCallback _cb, void *_cbdata)
        : recurse(_recurse), cb(_cb), cbdata(_cbdata) { }

    /** Returns watch recursion */
    bool recursive()
        { return recurse; }

    /** Notifies that a key has changed. */
    void notify(const UniConf &cfg)
        { cb(cfg, cbdata); }

    /** Equality test. */
    bool operator== (const UniWatch &other) const
    {
        return recurse  == other.recurse &&
            cb == other.cb && cbdata == other.cbdata;
    }
};
DeclareWvList(UniWatch);


/**
 * @internal
 * Data structure to track requested watches.
 */
class UniWatchTree : public UniConfTree<UniWatchTree>
{
public:
    UniWatchList watches;
    
    UniWatchTree(UniWatchTree *parent,
        const UniConfKey &key = UniConfKey::EMPTY)
        : UniConfTree<UniWatchTree>(parent, key) { }

    /** Returns true if the node should not be pruned. */
    bool isessential()
        { return haschildren() || ! watches.isempty(); }
};


/**
 * Represents the root of a hierarhical registry consisting of pairs
 * of UniConfKeys and associated string values.  * 
 *
 * Any number of data containers may be mounted into the tree at any
 * number of mount points to provide a backing store from which
 * registry keys and values are fetched and into which they are
 * stored.  Multiple data containers may be mounted at the same
 * location using standard unix semantics.
 *
 */
class UniConfRoot : public UniConf
{
    friend class UniConf;
    friend class UniConf::Iter;

    UniWatchTree watchroot;
    
    /** undefined. */
    UniConfRoot(const UniConfRoot &other);

public:
    /** Creates an empty UniConf tree with no mounted stores. */
    UniConfRoot() : UniConf(this), watchroot(NULL)
    {
        mounts.setcallback(UniConfGenCallback(this,
            &UniConfRoot::gen_callback), NULL);
    }

    /** Destroys the UniConf tree along with all uncommitted data. */
    ~UniConfRoot()
        { mounts.setcallback(UniConfGenCallback(), NULL); }

    /** 
     * Creates a new UniConf tree and mounts the given moniker at the root.
     * Since most people only want to mount one generator, this should save
     * a line of code here and there.
     */
    UniConfRoot(WvStringParm moniker, bool refresh = true)
        : UniConf(this), watchroot(NULL)
    {
        mounts.mount("/", moniker, refresh);
        mounts.setcallback(UniConfGenCallback(this,
            &UniConfRoot::gen_callback), NULL);
    }

    /** 
     * Creates a new UniConf tree and mounts the given generator at the root.
     * Since most people only want to mount one generator, this should save
     * a line of code here and there.
     */
    UniConfRoot(UniConfGen *gen, bool refresh = true)
        : UniConf(this), watchroot(NULL)
        { mounts.mountgen("/", gen, refresh); }

    /**
     * Requests notification when any of the keys covered by the
     * recursive depth specification change by invoking a callback.
     */
    void add_callback(const UniConfKey &key, const UniConfCallback &callback,
                      void *userdata, bool recurse = true);
    
    /**
     * Cancels notification requested using add_callback().
     */
    void del_callback(const UniConfKey &key, const UniConfCallback &callback,
                      void *userdata, bool recurse = true);

    /**
     * Requests notification when any of the keys covered by the
     * recursive depth specification change by setting a flag.
     */
    void add_setbool(const UniConfKey &key, bool *flag, bool recurse = true);

    /**
     * Cancels notification requested using add_setbool().
     */
    void del_setbool(const UniConfKey &key, bool *flag, bool recurse = true);

private:
    /**
     * Checks a branch of the watch tree for notification candidates.
     *   node - the current node
     *   key - the key that changed
     *   segleft - the number of segments left in the key (possibly negative)
     */
    void check(UniWatchTree *node, const UniConfKey &key, int segleft);

    /**
     * Recursively checks a branch of the watch tree for notification candidates.
     *   node - the current node
     *   key - the key that changed
     */
    void deletioncheck(UniWatchTree *node, const UniConfKey &key);

    /** Prunes a branch of the watch tree. */
    void prune(UniWatchTree *node);
    
    /** Internal callback for setbool style notifications. */
    void setbool_callback(const UniConf &cfg, void *userdata);

    /** Callback from UniMountTreeGen */
    void gen_callback(const UniConfKey &key, WvStringParm value,
                      void *userdata);

protected:
    UniMountGen mounts;
};

#endif //__UNICONFROOT_H
