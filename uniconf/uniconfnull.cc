/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator that is always empty and rejects changes.
 */
#include "uniconfnull.h"
#include "wvmoniker.h"

static UniConfGen *creator(WvStringParm, IObject *, void *)
{
    return new UniConfNullGen();
}

static WvMoniker<UniConfGen> reg("null", creator);


UniConfNullGen::UniConfNullGen()
{
}


UniConfNullGen::~UniConfNullGen()
{
}


WvString UniConfNullGen::get(const UniConfKey &key)
{
    return WvString::null;
}


bool UniConfNullGen::set(const UniConfKey &key, WvStringParm value)
{
    return false;
}


bool UniConfNullGen::zap(const UniConfKey &key)
{
    return false;
}


bool UniConfNullGen::haschildren(const UniConfKey &key)
{
    return false;
}


UniConfNullGen::Iter *UniConfNullGen::iterator(const UniConfKey &key)
{
    return new NullIter();
}
