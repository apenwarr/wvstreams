/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A read only generator wrapper.
 */
#include "unireadonlygen.h"
#include "wvmoniker.h"

// if 'obj' is non-NULL and is a UniConfGen, wrap that; otherwise wrap the
// given moniker.
static IUniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    IUniConfGen *gen = NULL;
    
    if (obj)
	gen = mutate<IUniConfGen>(obj);
    if (!gen)
	gen = wvcreate<IUniConfGen>(s);
    
    return new UniReadOnlyGen(gen);
}

static WvMoniker<IUniConfGen> reg("readonly", creator);
