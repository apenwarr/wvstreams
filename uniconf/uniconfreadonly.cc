/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
 
/** \file
 * A read only generator wrapper.
 */
#include "uniconfreadonly.h"

/***** UniConfReadOnlyGen *****/

UniConfReadOnlyGen::UniConfReadOnlyGen(
    const UniConfLocation &_location, UniConfGen *_inner) :
    UniConfGen(_location), inner(_inner)
{
}


UniConfReadOnlyGen::~UniConfReadOnlyGen()
{
    delete inner;
}



/***** UniConfReadOnlyGenFactory *****/

UniConfGen *UniConfReadOnlyGenFactory::newgen(
    const UniConfLocation &location, UniConf *top)
{
    UniConfGen *inner = UniConfGenFactoryRegistry::instance()->
        newgen(location.payload(), top);
    if (inner)
        return new UniConfReadOnlyGen(location, inner);
    return NULL;
}
