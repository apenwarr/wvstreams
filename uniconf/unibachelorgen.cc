/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 *
 * A UniConf generator that refuses to commit() or refresh().  This is
 * useful in blocking propogation of these messages upstream.
 */

#include "unibachelorgen.h"
#include "wvmoniker.h"


static IUniConfGen *creator(WvStringParm s)
{
    return new UniBachelorGen(s);
}

static const UUID uuid = {0x44b2e49f, 0x3c42, 0x4915,
			  {0xb1, 0xc7, 0x1f, 0xdb, 0xf9, 0x35, 0x5e, 0xde}};
static WvMoniker<IUniConfGen> moniker("bachelor", uuid, creator);

UniBachelorGen::UniBachelorGen(IUniConfGen *inner)
    : UniFilterGen(inner)
{
}

UniBachelorGen::UniBachelorGen(WvStringParm moniker)
    : UniFilterGen(NULL)
{
    setinner(wvcreate<IUniConfGen>(moniker));
}

void UniBachelorGen::commit()
{
}


bool UniBachelorGen::refresh()
{
    return false;
}
