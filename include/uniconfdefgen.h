/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConfDefGen is a UniConfGen for retrieving data with defaults
 */

#ifndef __UNICONFDEFGEN_H
#define __UNICONFDEFGEN_H

#include "uniconfgen.h"

class UniConfDefGen : public UniConfFilterGen
{
    WvString finddefault(UniConfKey key, UniConfKey keypart = "");

public:
    UniConfDefGen(UniConfGen *gen) : UniConfFilterGen(gen) { }

    /***** Overridden members *****/

    virtual WvString get(const UniConfKey &key);
};

#endif // __UNICONFDEFGEN_H
