/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Defines a UniConfGen that manages a tree of UniConfGen instances.
 */
#include "unimountgen.h"
#include "wvmoniker.h"

/***** UniMountGen *****/

WvString UniMountGen::get(const UniConfKey &key)
{
    if (!findgen(key))
        return WvString::null;

    return foundgen->get(trimkey(key));
}


void UniMountGen::set(const UniConfKey &key, WvStringParm value)
{
    if (!findgen(key))
        return;

    foundgen->set(trimkey(key), value);
}


bool UniMountGen::exists(const UniConfKey &key)
{
    if (!findgen(key))
        return false;

    return foundgen->exists(trimkey(key));
}


bool UniMountGen::haschildren(const UniConfKey &key)
{
    if (!findgen(key))
        return false;

    if (foundgen->haschildren(trimkey(key)));
        return true;

    //FIXME: Perhaps this should be optimized later
    MountList::Iter i(mounts);

    for (i.rewind(); i.next(); )
    {
        if (key.suborsame(i->key))
            return true;

        if (i->gen == foundgen)
            break;
    }

    return false;
}


bool UniMountGen::refresh()
{
    hold_delta();

    bool result = true;

    MountList::Iter i(mounts);

    for (i.rewind(); i.next();)
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


UniConfGen *UniMountGen::mount(const UniConfKey &key,
    WvStringParm moniker, bool refresh)
{

    UniConfGen *gen = wvcreate<UniConfGen>(moniker);
    if (gen)
        mountgen(key, gen, refresh); // assume always succeeds for now

    assert(gen && "Moniker doesn't get us a generator!");
    return gen;
}


UniConfGen *UniMountGen::mountgen(const UniConfKey &key,
    UniConfGen *gen, bool refresh)
{
    UniGenMount *newgen = new UniGenMount(gen, key);
    gen->setcallback(UniConfGenCallback(this,
        &UniMountGen::gencallback), &newgen->key);

    hold_delta();

    makemount(key);

    if (gen && refresh)
        gen->refresh();

    mounts.prepend(newgen, true);
    
    unhold_delta();
    return gen;
}


void UniMountGen::unmount(UniConfGen *gen, bool commit)
{
    MountList::Iter i(mounts);

    i.rewind();
    while (i.next() && i->gen != gen);

    if (i->gen != gen)
        return;

    hold_delta();
    
    if (commit)
        gen->commit();
    gen->setcallback(UniConfGenCallback(), NULL);

    UniConfKey key(i->key);
    UniConfGen *next = NULL;

    // Find the first generator mounted past the one we're removing (if any).
    // This way we can make sure that each generator still has keys leading up
    // to it (in case they lost their mountpoint due to the unmounted generator)
    i.xunlink();
    if (i.next())
        next = i->gen;

    i.rewind();
    while (i.next() && i->gen != next)
    {
        if (key.suborsame(i->key) && key != i->key)
            makemount(i->key);
    } 

    unhold_delta();
}


UniConfGen *UniMountGen::whichmount(const UniConfKey &key,
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
    if (findgen(key))
        return foundgen->iterator(trimkey(key));
    return new NullIter;
}


bool UniMountGen::findgen(const UniConfKey &key)
{
    MountList::Iter i(mounts);

    // Find the needed generator and keep it as a lastfound
    for (i.rewind(); i.next(); )
    {
        if (i->key.suborsame(key))
        {
            foundgen = i->gen;
            foundkey = i->key;
            return true;
        }
    } 

    return false;
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
    if (!findgen(points.removelast()))
        return;

    if (foundgen->get(trimkey(key)).isnull())
        foundgen->set(trimkey(key), "");
}
