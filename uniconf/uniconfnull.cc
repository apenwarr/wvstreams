/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
 
/** \file
 * A generator that is always empty and rejects changes.
 */
#include "uniconfnull.h"

/***** UniConfNullGen *****/

UniConfNullGen::UniConfNullGen()
{
}


UniConfNullGen::~UniConfNullGen()
{
}


UniConfLocation UniConfNullGen::location() const
{
    return UniConfLocation("null://");
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



/***** UniConfNullGenFactory *****/

UniConfNullGen *UniConfNullGenFactory::newgen(
    const UniConfLocation &location)
{
    return new UniConfNullGen();
}
