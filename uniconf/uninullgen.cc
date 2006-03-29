/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator that is always empty and rejects changes.
 */
#include "uninullgen.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"

WV_LINK(UniNullGen);


static IUniConfGen *creator(WvStringParm)
{
    return new UniNullGen();
}

static WvMoniker<IUniConfGen> reg("null", creator);
