/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen that makes everything slow.  See unislowgen.h.
 */
#include "unislowgen.h"
#include "wvmoniker.h"
#include <unistd.h>


// if 'obj' is non-NULL and is a UniConfGen, wrap that; otherwise wrap the
// given moniker.
static IUniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    IUniConfGen *gen = NULL;

    if (obj)
        gen = mutate<IUniConfGen>(obj);
    if (!gen)
        gen = wvcreate<IUniConfGen>(s);

    return new UniSlowGen(gen);
}

static WvMoniker<IUniConfGen> reg("slow", creator);


UniSlowGen::UniSlowGen(IUniConfGen *inner) : UniFilterGen(inner)
{
    slowcount = 0;
}


UniSlowGen::~UniSlowGen()
{
    fprintf(stderr, "%p: UniSlowGen: ran a total of %d slow operations.\n",
	    this, how_slow());
}


void UniSlowGen::commit()
{
    be_slow("commit()");
    UniFilterGen::commit();
}


bool UniSlowGen::refresh()
{
    be_slow("refresh()");
    return UniFilterGen::refresh();
}


WvString UniSlowGen::get(const UniConfKey &key)
{
    be_slow("get(%s)", key);
    return UniFilterGen::get(key);
}


bool UniSlowGen::exists(const UniConfKey &key)
{
    be_slow("exists(%s)", key);
    return UniFilterGen::exists(key);
}


bool UniSlowGen::haschildren(const UniConfKey &key)
{
    be_slow("haschildren(%s)", key);
    return UniFilterGen::haschildren(key);
}


UniConfGen::Iter *UniSlowGen::iterator(const UniConfKey &key)
{
    be_slow("iterator(%s)", key);
    return UniFilterGen::iterator(key);
}


UniConfGen::Iter *UniSlowGen::recursiveiterator(const UniConfKey &key)
{
    be_slow("recursiveiterator(%s)", key);
    return UniFilterGen::recursiveiterator(key);
}


void UniSlowGen::be_slow(WvStringParm what)
{
    fprintf(stderr, "%p: UniSlowGen: slow operation: %s\n",
	    this, what.cstr());
    // sleep(1);
    slowcount++;
}


