/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * UniDefGen is a UniConfGen for retrieving data with defaults
 *
 */

#include "unidefgen.h"
#include "wvmoniker.h"

// if 'obj' is non-NULL and is a UniConfGen, wrap that; otherwise wrap the
// given moniker.
static UniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    UniConfGen *gen = NULL;

    if (obj)
        gen = mutate<UniConfGen>(obj);
    if (!gen)
        gen = wvcreate<UniConfGen>(s);

    return new UniDefGen(gen);
}

static WvMoniker<UniConfGen> reg("default", creator);


WvString UniDefGen::get(const UniConfKey &key)
{
    return finddefault(key);
}


WvString UniDefGen::finddefault(UniConfKey key, UniConfKey keypart)
{
    if (key.isempty())
        return UniFilterGen::get(keypart);

    WvString cur = key.first();
    key = key.removefirst();

    cur = finddefault(key, WvString("%s/%s", keypart, cur));
    if (!!cur)
        return cur;
    else
        return finddefault(key, WvString("%s/*", keypart));
}
