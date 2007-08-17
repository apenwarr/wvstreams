/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen that makes everything slow.  See unislowgen.h.
 */
#include "unislowgen.h"
#include "wvmoniker.h"
#ifndef _MSC_VER // FIXME:WLACH Is unistd even needed here?!
#include <unistd.h>
#endif

static IUniConfGen *creator(WvStringParm s, IObject *_obj)
{
    return new UniSlowGen(wvcreate<IUniConfGen>(s, _obj));
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


