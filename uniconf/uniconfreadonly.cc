/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
 
/** \file
 * A read only generator wrapper.
 */
#include "uniconfreadonly.h"

/***** UniConfReadOnlyGen *****/

UniConfReadOnlyGen::UniConfReadOnlyGen(UniConfGen *inner) :
    UniConfFilterGen(inner)
{
}


UniConfLocation UniConfReadOnlyGen::location() const
{
    return WvString("readonly://%s", UniConfFilterGen::location());
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



/***** UniConfReadOnlyGenFactory *****/

UniConfReadOnlyGen *UniConfReadOnlyGenFactory::newgen(
    const UniConfLocation &location)
{
    UniConfGen *inner = UniConfGenFactoryRegistry::instance()->
        newgen(location.payload());
    if (inner)
        return new UniConfReadOnlyGen(inner);
    return NULL;
}
