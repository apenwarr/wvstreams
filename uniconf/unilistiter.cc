/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A simple implementation of a UniConfGen iterator.  See unilistiter.h.
 */
#include "unilistiter.h"

UniListIter::UniListIter(IUniConfGen *_gen)
    : ki(keys), vi(values)
{
    gen = _gen;
}


void UniListIter::add(const UniConfKey &k, WvStringParm v)
{ 
    UniConfKey *nk = new UniConfKey(k);
    keys.append(nk, true);
    keylook.add(nk, false);
    if (!v.isnull())
        values.append(new WvString(scache.get(v)), true);
}


void UniListIter::autofill(IUniConfGen::Iter *_source)
{
    IUniConfGen::Iter &source(*_source);
    for (source.rewind(); source.next(); )
	add(source.key(), source.value());
}


void UniListIter::rewind()
{
    ki.rewind();
    vi.rewind();
}


bool UniListIter::next()
{
    if (vi.cur())
	vi.next();
    return ki.next();
}


UniConfKey UniListIter::key() const
{
    return *ki;
}


WvString UniListIter::value() const
{
    if (vi.cur())
	return *vi;
    else
	return gen->get(*ki);
}
