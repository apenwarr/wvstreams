/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Defines some important UniConf types.
 */
#include "uniconfdefs.h"
#include "strutils.h"
#include <assert.h>

/***** UniConfDepth *****/

namespace UniConfDepth
{

static const char *names[NUM_DEPTHS + 1] = {
    "zero", "one", "infinite", "children", "descendents", NULL
};


const char *nameof(Type depth)
{
    return names[depth];
}


Type fromname(const char *name)
{
    return Type(::lookup(name, names));
}

}; // namespace
