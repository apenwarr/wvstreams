/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A simple implementation of a UniConfGen iterator.  See unilistiter.h.
 */
#include "unilistiter.h"

const WvString UniListIter::noval = "whatever";


UniSmartKey::UniSmartKey(const UniSmartKey *_parent, const UniConfKey &_child)
    : parent(_parent), child(_child)
{ 
    // nothing special
}


UniConfKey UniSmartKey::key() const
{
    if (parent)
	return UniConfKey(parent->key(), child);
    else
	return child;
}
    

bool UniSmartKey::operator== (const UniSmartKey &k) const
{
    return &k == this || k.key() == key();
}


unsigned WvHash(const UniSmartKey &k)
{
    return ::WvHash(k.key());
}


UniListIter::UniListIter(IUniConfGen *_gen)
    : ki(keys), vi(values)
{
    gen = _gen;
    no_more_values = false;
}


UniSmartKey *UniListIter::new_smart_key(const UniConfKey &k)
{
    UniSmartKey *s = keylook[UniSmartKey(NULL, k.removelast())];
    if (s)
	return new UniSmartKey(s, scache.get(k.last()));
    else
	return new UniSmartKey(NULL, scache.get(k));
}
    

void UniListIter::add(WvStringParm k, WvStringParm v)
{ 
    UniSmartKey *sk = new_smart_key(k);
    // UniSmartKey *sk = new UniSmartKey(NULL, k);
    keys.append(sk, true);
    keylook.add(sk, false);
    if (v.cstr() != noval.cstr())
    {
	assert(!no_more_values);
	values.append(new WvString(scache.get(v)), true);
    }
    else
	no_more_values = true;
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
    return ki->key();
}


WvString UniListIter::value() const
{
    if (vi.cur())
	return *vi;
    else
	return gen->get(ki->key());
}
