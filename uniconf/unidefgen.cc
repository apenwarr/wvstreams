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
    WvString tmp_key(key), tmp("");
    char *p = tmp_key.edit();

    tmp.setsize(strlen(tmp_key) * 2);
    char *q = tmp.edit();
    *q = '\0';

    WvString result;
    finddefault(p, q, result);
    return result;
}


void UniDefGen::finddefault(char *p, char *q, WvString &result)
{
    if (!p)
    {
        result = UniFilterGen::get(++q);
        return;
    }

    char *r = strchr(p, '/');
    if (r)
        *r++ = '\0';

    char *s = strchr(q, '\0');
    *s++ = '/';
    *s = 0;
    q = strcat(q, p);

    finddefault(r, q, result);

    if (!result.isnull())
        return;
    else
    {
        *s++ = '*';
        *s = '\0';
        finddefault(r, q, result);
    }

    if (r)
        *--r = '/';
}
