/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
 
/** \file
 * A generator that is always empty and rejects changes.
 */
#include "uniconfnull.h"

/***** UniConfNullGen *****/

UniConfNullGen::UniConfNullGen() :
    UniConfGen(UniConfLocation("null://"))
{
}


UniConfNullGen::~UniConfNullGen()
{
}



/***** UniConfNullGenFactory *****/

UniConfGen *UniConfNullGenFactory::newgen(
    const UniConfLocation &location, UniConf *top)
{
    return new UniConfNullGen();
}
