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

static const char *depthnames[UniConfDepth::NUM_DEPTHS] = {
    "zero", "one", "infinite", "children", "descendents"
};


const char *UniConfDepth::nameof(UniConfDepth::Type depth)
{
    return depthnames[depth];
}


UniConfDepth::Type UniConfDepth::fromname(const char *name)
{
    return Type(lookup(name, depthnames));
}
