/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines the root management class for UniConf.  To create any kind of
 * UniConf tree, you'll need one of these.
 */
#include "uniconfroot.h"
#include "wvlinkerhack.h"

WV_LINK_TO(UniGenHack);


UniConfRoot::UniConfRoot():
    UniConf(this),
    watchroot(NULL)
{
    mounts.add_callback(this, wv::bind(&UniConfRoot::gen_callback, this,
				       _1, _2));
}


UniConfRoot::UniConfRoot(WvStringParm moniker, bool refresh):
    UniConf(this),
    watchroot(NULL)
{
    mounts.mount("/", moniker, refresh);
    mounts.add_callback(this, wv::bind(&UniConfRoot::gen_callback, this,
				       _1, _2));
}


UniConfRoot::UniConfRoot(UniConfGen *gen, bool refresh):
    UniConf(this),
    watchroot(NULL)
{
    mounts.mountgen("/", gen, refresh);
    mounts.add_callback(this, wv::bind(&UniConfRoot::gen_callback, this,
				       _1, _2));
}


// make sure the given subtree of callback information is empty
static bool watchout(UniWatchInfoTree *t)
{
    bool fail = false;
    
    UniWatchInfoTree::Iter i(*t);
    for (i.rewind(); i.next(); )
    {
	UniWatchInfoTree *w = i.ptr();
	
	if (w->haschildren())
	    if (watchout(w))
		fail = true;
	
	if (!w->watches.isempty())
	{
	    fail = true;
	    if (1)
		fprintf(stderr, "Remaining watch: '%s' (%d)\n",
			w->fullkey().printable().cstr(), w->watches.count());
	}
    }
    
    return fail;
}


UniConfRoot::~UniConfRoot()
{
    // first, unmount everything.  Some of the mounts might have waiting
    // callbacks.  (I hope not, but... things like UniUnwrapGen might get
    // confusing.)
    mounts.zap();
    
    // if the list of callbacks is non-empty, someone is either very buggy
    // (they disappeared without deleting their callback, so they could cause
    // crashes), or they're not getting what they expected (we disappeared
    // before they did, so they won't be getting their callback).
    assert(!watchout(&watchroot));
    
    mounts.del_callback(this);
}


void UniConfRoot::add_callback(void *cookie, const UniConfKey &key,
			       const UniConfCallback &callback, bool recurse)
{
    UniWatchInfo *w = new UniWatchInfo(cookie, recurse, callback);

    UniWatchInfoTree *node = &watchroot;
    
    UniConfKey::Iter i(key);
    for (i.rewind(); i.next(); )
    {
        UniWatchInfoTree *prev = node;
        node = node->findchild(i());
        if (!node)
            node = new UniWatchInfoTree(prev, i());
    }
    node->watches.append(w, true);
}


void UniConfRoot::del_callback(void *cookie, const UniConfKey &key,
			       bool recurse)
{
    UniWatchInfoTree *node = watchroot.find(key);
    if (node)
    {
        UniWatchInfoList::Iter i(node->watches);
        for (i.rewind(); i.next(); )
        {
	    // remove the watch if it matches
            if (i->cookie == cookie && i->recurse == recurse)
	    {
                i.xunlink();
		break;
	    }
        }
        // prune the branch if needed
        prune(node);
    }
}


void UniConfRoot::add_setbool(const UniConfKey &key, bool *flag, bool recurse)
{
    add_callback(flag, key, wv::bind(&UniConfRoot::setbool_callback, flag,
				     _1, _2),
		 recurse);
}


void UniConfRoot::del_setbool(const UniConfKey &key, bool *flag, bool recurse)
{
    del_callback(flag, key, recurse);
}


void UniConfRoot::check(UniWatchInfoTree *node,
			const UniConfKey &key, int segleft)
{
    UniWatchInfoList::Iter i(node->watches);
    for (i.rewind(); i.next(); )
    {
        if (!i->recursive() && segleft > 0)
            continue;

        i->notify(UniConf(this, key.removelast(segleft)), key.last(segleft));
    }
}


void UniConfRoot::deletioncheck(UniWatchInfoTree *node, const UniConfKey &key)
{
    UniWatchInfoTree::Iter i(*node);
    for (i.rewind(); i.next(); )
    {
        UniWatchInfoTree *w = i.ptr();
        UniConfKey subkey(key, w->key());
        
        // pretend that we wiped out just this key
        check(w, subkey, 0);
        deletioncheck(w, subkey);
    }
}


void UniConfRoot::prune(UniWatchInfoTree *node)
{
    while (node != & watchroot && ! node->isessential())
    {
        UniWatchInfoTree *next = node->parent();
        delete node;
        node = next;
    }
}


void UniConfRoot::gen_callback(const UniConfKey &key, WvStringParm value)
{
    hold_delta();
    UniWatchInfoTree *node = & watchroot;
    int segs = key.numsegments();

    // check root node
    check(node, key, segs);

    // look for watches on key and its ancestors
    for (int s = 0; s < segs; )
    {
        node = node->findchild(key.segment(s));
        s++;
        if (!node)
            goto done; // no descendents so we can stop
        check(node, key, segs - s);
    }

    // look for watches on descendents of key if node was deleted
    if (value.isnull())
        deletioncheck(node, key);
    
done:
    unhold_delta();
}
