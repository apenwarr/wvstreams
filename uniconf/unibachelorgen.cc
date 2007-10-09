/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005 Net Integration Technologies, Inc.
 *
 * A UniConf generator that refuses to commit() or refresh().  This is
 * useful in blocking propogation of these messages upstream.
 */

#include "unibachelorgen.h"
#include "wvmoniker.h"


static IUniConfGen *creator(WvStringParm s, IObject *_obj)
{
    return new UniBachelorGen(wvcreate<IUniConfGen>(s, _obj));
}

static WvMoniker<IUniConfGen> moniker("bachelor", creator);

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
