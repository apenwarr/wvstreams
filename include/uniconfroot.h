/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines the root management class for UniConf.
 */
#ifndef __UNICONFROOT_H
#define __UNICONFROOT_H

#include "uniconf.h"
#include "unimounttreegen.h"

/**
 * @internal
 * Holds information about a single watch.
 */
class UniWatch
{
    UniConfDepth::Type xdepth;
    UniConfCallback cb;
    void *cbdata;

public:
    UniWatch(UniConfDepth::Type depth, UniConfCallback _cb, void *_cbdata)
        : xdepth(depth), cb(_cb), cbdata(_cbdata) { }

    /** Returns watch recursion depth. */
    UniConfDepth::Type depth()
        { return xdepth; }

    /** Notifies that a key has changed. */
    void notify(const UniConf &cfg)
        { cb(cfg, cbdata); }

    /** Equality test. */
    bool operator== (const UniWatch &other) const
    {
        return xdepth == other.xdepth &&
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
    
    UniWatchTree(UniWatchTree *parent, const UniConfKey &key)
        : UniConfTree<UniWatchTree>(parent, key) { }

    /** Returns true if the node should not be pruned. */
    bool isessential()
        { return haschildren() || ! watches.isempty(); }
};


/**
 * The UniConfRoot implementation.
 *
 * Wires together all of the bits and pieces that make up the core structure
 * of a UniConf tree.  These pieces are factored to make them easier to
 * understand as independent units.  They may also be used independently
 * to achieve a variety of interesting effects.
 * 
 * For tree contents and mounting support see UniMountTreeGen.
 */
class UniConfRootImpl : public UniMountTreeGen
{
    UniWatchTree watchroot;
    
    /** undefined. */
    UniConfRootImpl(const UniConfRootImpl &other);

public:
    /** Creates an empty UniConf tree with no mounted stores. */
    UniConfRootImpl();

    /** Destroys the UniConf tree along with all uncommitted data. */
    ~UniConfRootImpl();
    
    /**
     * Requests notification when any of the keys covered by the
     * recursive depth specification change by invoking a callback.
     */
    void add_callback(const UniConfKey &key,
        const UniConfCallback &callback, void *userdata,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);
    
    /**
     * Cancels notification requested using add_callback().
     */
    void del_callback(const UniConfKey &key,
        const UniConfCallback &callback, void *userdata,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    /**
     * Requests notification when any of the keys covered by the
     * recursive depth specification change by setting a flag.
     */
    void add_setbool(const UniConfKey &key, bool *flag,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

    /**
     * Cancels notification requested using add_setbool().
     */
    void del_setbool(const UniConfKey &key, bool *flag,
        UniConfDepth::Type depth = UniConfDepth::INFINITE);

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
     *   segleft - the number of segments left in the key (possibly negative)
     */
    void recursivecheck(UniWatchTree *node,
        const UniConfKey &key, int segleft);

    /** Prunes a branch of the watch tree. */
    void prune(UniWatchTree *node);
    
    /** Internal callback for setbool style notifications. */
    void setbool_callback(const UniConf &cfg, void *userdata);

    /** Callback from UniMountTreeGen */
    void gen_callback(UniConfGen *gen, const UniConfKey &key,
        void *userdata);
};

#endif //__UNICONFROOT_H
