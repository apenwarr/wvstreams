/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a UniConfGen that manages a tree of UniConfGen instances.
 */
#include "unimountgen.h"
#include "wvmoniker.h"
#include <assert.h>

/***** UniMountGen *****/

WvString UniMountGen::get(const UniConfKey &key)
{
    UniGenMount *found = findmount(key);
    if (!found)
        return WvString::null;

    return found->gen->get(trimkey(found->key, key));
}


void UniMountGen::set(const UniConfKey &key, WvStringParm value)
{
    UniGenMount *found = findmount(key);
    if (!found)
        return;

    found->gen->set(trimkey(found->key, key), value);
}


bool UniMountGen::exists(const UniConfKey &key)
{
    UniGenMount *found = findmount(key);
//    fprintf(stdout, "exists:found %p\n", found);
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
	// the list is sorted innermost-first.  So if we find the key
	// we started with, we've finished searching all children of it.
        if (found && (i->gen == found->gen))
            break;

        if (key.suborsame(i->key))
        {
//            fprintf(stdout, "%s has_subkey %s : true\n", key.printable().cstr(), 
//                    i->key.printable().cstr());
            return true;
        }
    }

//    fprintf(stdout, "has_subkey false\n");
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

    // assert(gen && "Moniker doesn't get us a generator!");
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
    gen->setcallback(UniConfGenCallback(this,
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
    gen->setcallback(UniConfGenCallback(), NULL);

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


IUniConfGen *UniMountGen::whichmount(const UniConfKey &key,
				    UniConfKey *mountpoint)
{
    MountList::Iter i(mounts);

    for (i.rewind(); i.next(); )
    {
        if (i->key.suborsame(key))
        {
            if (mountpoint)
                *mountpoint = key;
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


UniMountGen::Iter *UniMountGen::iterator(const UniConfKey &key)
{
    UniGenMount *found = findmount(key);
    if (found)
        return found->gen->iterator(trimkey(found->key, key));
    return NULL;
}


UniMountGen::Iter *UniMountGen::recursiveiterator(const UniConfKey &key)
{
    UniGenMount *found = findmount(key);
    if (found)
        return found->gen->recursiveiterator(trimkey(found->key, key));
    return NULL;
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
