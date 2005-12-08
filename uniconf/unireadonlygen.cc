/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A read only generator wrapper.
 */
#include "unireadonlygen.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"

WV_LINK(UniReadOnlyGen);


static IUniConfGen *creator(WvStringParm s)
{
    return new UniReadOnlyGen(wvcreate<IUniConfGen>(s));
}

static WvMoniker<IUniConfGen> reg("readonly", creator);
