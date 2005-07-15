/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a UniConfGen that manages a tree of UniConfGen instances.
 */
#include "unimountgen.h"
#include "wvmoniker.h"
#include "wvhash.h"
#include "wvstrutils.h"
#include "unilistiter.h"
#include "wvstringtable.h"
#include <assert.h>

/***** UniMountGen *****/

UniMountGen::UniMountGen()
{
    // nothing special
}


UniMountGen::~UniMountGen()
{
    zap();
}


WvString UniMountGen::get(const UniConfKey &key)
{
    UniGenMount *found = findmount(key);
    if (!found)
    {
        // if there are keys that _do_ have a mount under this one,
        // then we consider it to exist (as a key with a blank value)
        if (has_subkey(key, NULL))
            return "";

        return WvString::null;
    }

    return found->gen->get(trimkey(found->key, key));
}


void UniMountGen::set(const UniConfKey &key, WvStringParm value)
{
    UniGenMount *found = findmount(key);
    if (!found)
        return;
    found->gen->set(trimkey(found->key, key), value);
}


struct UniMountGen::UniGenMountPairs
{
    UniGenMount *mount;
    UniConfPairList pairs;

    UniGenMountPairs(UniGenMount *_mount)
	: mount(_mount)
    {
    }
};


void UniMountGen::setv(const UniConfPairList &pairs)
{
    UniGenMountPairsDict mountpairs(mounts.count());

    {
	MountList::Iter m(mounts);
	for (m.rewind(); m.next(); )
	    mountpairs.add(new UniGenMountPairs(m.ptr()), true);
    }

    {
	UniConfPairList::Iter pair(pairs);
	for (pair.rewind(); pair.next(); )
	{
	    UniGenMount *found = findmount(pair->key());
	    if (!found)
		continue;
	    UniConfPair *trimmed = new UniConfPair(trimkey(found->key,
							   pair->key()),
						   pair->value());
	    mountpairs[found->key]->pairs.add(trimmed, true);
	}
    }

    UniGenMountPairsDict::Iter i(mountpairs);
    for (i.rewind(); i.next(); )
	i->mount->gen->setv(i->pairs);
}


bool UniMountGen::exists(const UniConfKey &key)
{
    UniGenMount *found = findmount(key);
    //fprintf(stdout, "unimountgen:exists:found %p\n", found);
    if (found && found->gen->exists(trimkey(found->key, key)))
        return true;
    else
        //if there's something mounted and set on a subkey, this key must 
        //*exist* along the way
        return has_subkey(key, found);
}


bool UniMountGen::haschildren(const UniConfKey &key)
{
    UniGenMount *found = findmount(key);
//    fprintf(stdout, "haschildren:found %p\n", found);
    if (found && found->gen->haschildren(trimkey(found->key, key)))
        return true;

    // if we get here, the generator we used didn't have a subkey.  We want
    // to see if there's anyone mounted at a subkey of the requested key; if
    // so, then we definitely have a subkey.
    return has_subkey(key, found);
}


bool UniMountGen::has_subkey(const UniConfKey &key, UniGenMount *found)
{
    MountList::Iter i(mounts);
    for (i.rewind(); i.next(); )
    {
        if (key.suborsame(i->key) && key < i->key)
        {
            //fprintf(stdout, "%s has_subkey %s : true\n", key.printable().cstr(), 
            //        i->key.printable().cstr());            
            return true;
        }

	// the list is sorted innermost-first.  So if we find the key
	// we started with, we've finished searching all children of it.
        if (found && (i->gen == found->gen))
            break;
    }

    //fprintf(stdout, "%s has_subkey false \n", key.printable().cstr());
    return false;
}

bool UniMountGen::refresh()
{
    hold_delta();

    bool result = true;

    MountList::Iter i(mounts);
    for (i.rewind(); i.next(); )
        result = result && i->gen->refresh();

    unhold_delta();
    return result;
}


void UniMountGen::commit()
{
    hold_delta();

    MountList::Iter i(mounts);
    for (i.rewind(); i.next();)
        i->gen->commit();

    unhold_delta();
}


IUniConfGen *UniMountGen::mount(const UniConfKey &key,
			       WvStringParm moniker, bool refresh)
{
    IUniConfGen *gen = wvcreate<IUniConfGen>(moniker);
    if (gen)
        mountgen(key, gen, refresh); // assume always succeeds for now
#if DEBUG
    assert(gen && "Moniker doesn't get us a generator!");
#endif
    if (gen && !gen->exists("/"))
        gen->set("/", "");
    return gen;
}


IUniConfGen *UniMountGen::mountgen(const UniConfKey &key,
				   IUniConfGen *gen, bool refresh)
{
    if (!gen)
	return NULL;
    
    UniGenMount *newgen = new UniGenMount(gen, key);
    gen->add_callback(this, UniConfGenCallback(this,
				&UniMountGen::gencallback), &newgen->key);

    hold_delta();
    delta(key, WvString());

    makemount(key);

    if (gen && refresh)
        gen->refresh();

    mounts.prepend(newgen, true);
    
    delta(key, get(key));
    unhold_delta();
    if (!gen->exists("/"))
        gen->set("/", "");
    return gen;
}


void UniMountGen::unmount(IUniConfGen *gen, bool commit)
{
    if (!gen)
	return;
    
    MountList::Iter i(mounts);

    for (i.rewind(); i.next() && i->gen != gen; )
	;

    if (i->gen != gen)
        return;

    hold_delta();
    
    if (commit)
        gen->commit();
    gen->del_callback(this);

    UniConfKey key(i->key);
    IUniConfGen *next = NULL;

    delta(key, WvString());

    // Find the first generator mounted past the one we're removing (if
    // any). This way we can make sure that each generator still has keys
    // leading up to it (in case they lost their mountpoint due to the
    // unmounted generator)
    i.xunlink();
    if (i.next())
        next = i->gen;

    for (i.rewind(); i.next() && i->gen != next; )
    {
        if (key.suborsame(i->key) && key != i->key)
	{
            makemount(i->key);
	    delta(i->key, get(i->key));
	}
    } 

    unhold_delta();
}


void UniMountGen::zap()
{
    while (!mounts.isempty())
	unmount(mounts.first()->gen, false);
}


IUniConfGen *UniMountGen::whichmount(const UniConfKey &key,
				    UniConfKey *mountpoint)
{
    MountList::Iter i(mounts);

    for (i.rewind(); i.next(); )
    {
        if (i->key.suborsame(key))
        {
            if (mountpoint)
                *mountpoint = i->key;
            return i->gen;
        }
    }

    return NULL;
}


bool UniMountGen::ismountpoint(const UniConfKey &key)
{
    MountList::Iter i(mounts);

    for (i.rewind(); i.next(); )
    {
        if (i->key == key)
            return true;
    }

    return false;
}

static int wvstrcmp(const WvString *l, const WvString *r)
{
    return strcmp(*l, *r);
}

UniMountGen::Iter *UniMountGen::iterator(const UniConfKey &key)
{
    UniGenMount *found = findmount(key);
    if (found)
        return found->gen->iterator(trimkey(found->key, key));
    else
    {
	// deal with elements mounted on nothingness.
	// FIXME: this is really a hack, and should (somehow) be dealt with
	// in a more general way.
	ListIter *it = new ListIter(this);

	MountList::Iter i(mounts);
        WvStringTable t(10);
	for (i.rewind(); i.next(); )
	{
	    if (key.numsegments() < i->key.numsegments()
	      && key.suborsame(i->key))
            {
                // trim off any stray segments coming between the virtual
                // "key" we're iterating over and the mount
                UniConfKey k1 = i->key.first(key.numsegments() + 1);
                UniConfKey k2 = k1.last(); // final "key" should be size 1

                if (!t[k2])
                    t.add(new WvString(k2), true);
            }
	}
        WvStringTable::Sorter s(t, &::wvstrcmp);
        for (s.rewind(); s.next();)
	    it->add(*s);

	return it;
    }
}


// FIXME: this function will be rather slow if you try to iterate over multiple
// generators and the latency level is high (as is the case with e.g.: the tcp generator). 
// the fast path will only kick in if you iterate over a single generator.
UniMountGen::Iter *UniMountGen::recursiveiterator(const UniConfKey &key)
{
    UniGenMount *found = findmountunder(key);
    if (found)
        return found->gen->recursiveiterator(trimkey(found->key, key));
    else
	return UniConfGen::recursiveiterator(key);
}


UniMountGen::UniGenMount *UniMountGen::findmount(const UniConfKey &key)
{
    // Find the needed generator and keep it as a lastfound
    MountList::Iter i(mounts);
    for (i.rewind(); i.next(); )
    {
        if (i->key.suborsame(key))
	    return i.ptr();
    } 

    return NULL;
}


UniMountGen::UniGenMount *UniMountGen::findmountunder(const UniConfKey &key)
{
    UniMountGen::UniGenMount * foundmount = NULL;
    int num_found_mounts = 0;

    // Find the needed generator and keep it as a lastfound
    MountList::Iter i(mounts);
    for (i.rewind(); i.next(); )
    {
        // key lies beneath mount (only care about the first)
        if (i->key.suborsame(key) && !foundmount)
        {
            foundmount = i.ptr();
            num_found_mounts++;
        }
        // mount lies beneath key
        else if (key.suborsame(i->key))
        {
            num_found_mounts++;
        }
    }

    if (num_found_mounts == 1 && foundmount)
        return foundmount;

    return NULL;
}


void UniMountGen::gencallback(const UniConfKey &key, WvStringParm value,
                                  void *userdata)
{
    UniConfKey *base = static_cast<UniConfKey*>(userdata);
    delta(UniConfKey(*base, key), value);
}


void UniMountGen::makemount(const UniConfKey &key)
{
    // Create any keys needed leading up to the mount generator so that the
    // mountpoint exists
    UniConfKey::Iter i(key);
    UniConfKey points;

    for (i.rewind(); i.next(); )
    {
        points.append(*i);
        if (get(points).isnull())
            set(points, "");
    }

    // Set the mountpoint in the sub generator instead of on the generator
    // itself (since set will set it on the generator, instead of making the
    // mountpoint)
    UniGenMount *found = findmount(points.removelast());
    if (!found)
        return;

    if (found->gen->get(trimkey(found->key, key)).isnull())
        found->gen->set(trimkey(found->key, key), "");
}
