/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * UniConf key-value pair storage abstraction.
 */
#include "uniconfpair.h"

/***** UniConfPair *****/

UniConfPairDict null_UniConfPairDict(1);

UniConfPair::UniConfPair(const UniConfKey &key, WvStringParm value) :
    xkey(key), xvalue(value)
{
}


void UniConfPair::setvalue(WvStringParm value)
{
    xvalue = value;
}
