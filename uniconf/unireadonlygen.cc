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

static const UUID uuid = {0x8b93702c, 0x84cf, 0x4ee2,
			  {0xb7, 0x11, 0xf9, 0x03, 0x87, 0x38, 0xca, 0xd1}};
static WvMoniker<IUniConfGen> reg("readonly", uuid, creator);
