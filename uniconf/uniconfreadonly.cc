/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A read only generator wrapper.
 */
#include "uniconfreadonly.h"
#include "wvmoniker.h"

// if 'obj' is non-NULL and is a UniConfGen, wrap that; otherwise wrap the
// given moniker.
static UniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    UniConfGen *gen = NULL;
    
    if (obj)
	gen = mutate<UniConfGen>(obj);
    if (!gen)
	gen = wvcreate<UniConfGen>(s);
    
    return new UniConfReadOnlyGen(gen);
}

static WvMoniker<UniConfGen> reg("readonly", creator);


UniConfReadOnlyGen::UniConfReadOnlyGen(UniConfGen *inner) :
    UniConfFilterGen(inner)
{
}


bool UniConfReadOnlyGen::set(const UniConfKey &key, WvStringParm value)
{
    return false;
}


bool UniConfReadOnlyGen::zap(const UniConfKey &key)
{
    return false;
}


bool UniConfReadOnlyGen::commit(const UniConfKey &key, 
				UniConfDepth::Type depth)
{
    return false;
}
