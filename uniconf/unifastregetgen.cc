/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002-2005 Net Integration Technologies, Inc.
 * 
 * A lightweight but slightly dangerous version of UniCacheGen.
 */
#include <wvassert.h>

#include "unifastregetgen.h"
#include "uniconftree.h"
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

    return new UniFastRegetGen(gen);
}

static WvMoniker<IUniConfGen> reg("fast-reget", creator);


UniFastRegetGen::UniFastRegetGen(IUniConfGen *_inner)
    : UniFilterGen(_inner)
{
    tree = new UniConfValueTree(NULL, "/", UniFilterGen::get("/"));
}


UniFastRegetGen::~UniFastRegetGen()
{
    if (tree)
    {
	delete tree;
	tree = NULL;
    }
}


void UniFastRegetGen::gencallback(const UniConfKey &key, WvStringParm value)
{
    if (!tree)
	return;

    UniConfValueTree *t = tree->find(key);
    if (t) // never previously retrieved; don't cache it
	t->setvalue(value);
    UniFilterGen::gencallback(key, value);
}


WvString UniFastRegetGen::get(const UniConfKey &key)
{
    if (!tree)
    {
	wvassert(tree, "key: '%s'", key);
	abort();
    }

    UniConfValueTree *t = tree->find(key);
    if (!t)
    {
	get(key.removelast()); // guaranteed to create parent node
	t = tree->find(key.removelast());
	assert(t);
	
	WvString value;
	if (!t->value().isnull()) // if parent is null, child guaranteed null
	    value = UniFilterGen::get(key);
	new UniConfValueTree(t, key.last(), value);
	return value;
    }
    else
	return t->value();
}


bool UniFastRegetGen::exists(const UniConfKey &key)
{
    // even if inner generator has a more efficient version of exists(),
    // do it this way so we can cache the result.
    return !!get(key);
}


bool UniFastRegetGen::haschildren(const UniConfKey &key)
{
    if (!tree)
    {
	wvassert(tree, "key: '%s'", key);
	abort();
    }

    // if we already know the node is null, we can short circuit this one
    UniConfValueTree *t = tree->find(key);
    if (t && t->value().isnull())
	return false; // definitely no children
    return UniFilterGen::haschildren(key);
}
