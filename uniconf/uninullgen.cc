/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator that is always empty and rejects changes.
 */
#include "uninullgen.h"
#include "wvmoniker.h"

static UniConfGen *creator(WvStringParm, IObject *, void *)
{
    return new UniNullGen();
}

static WvMoniker<UniConfGen> reg("null", creator);
