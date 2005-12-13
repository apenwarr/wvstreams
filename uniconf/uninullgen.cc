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

static const UUID uuid = {0x8ac12f11, 0x205c, 0x4a17,
			  {0xa6, 0xa1, 0xd2, 0x9b, 0xa6, 0x84, 0x43, 0xef}};
static WvMoniker<IUniConfGen> reg("null", uuid, creator);
