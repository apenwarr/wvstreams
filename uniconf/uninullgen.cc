/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator that is always empty and rejects changes.
 */
#include "uninullgen.h"
#include "wvmoniker.h"

static UniConfGen *creator(WvStringParm, IObject *, void *)
{
    return new UniNullGen();
}

static WvMoniker<UniConfGen> reg("null", creator);


UniNullGen::UniNullGen()
{
}


UniNullGen::~UniNullGen()
{
}


WvString UniNullGen::get(const UniConfKey &key)
{
    return WvString::null;
}


bool UniNullGen::set(const UniConfKey &key, WvStringParm value)
{
    return false;
}


bool UniNullGen::zap(const UniConfKey &key)
{
    return false;
}


bool UniNullGen::haschildren(const UniConfKey &key)
{
    return false;
}


UniNullGen::Iter *UniNullGen::iterator(const UniConfKey &key)
{
    return new NullIter();
}
