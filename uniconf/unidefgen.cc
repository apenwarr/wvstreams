/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * UniDefGen is a UniConfGen for retrieving data with defaults
 *
 */

#include "unidefgen.h"
#include "wvmoniker.h"

#include <ctype.h>
#include <stdlib.h>

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
    finddefault(key, p, q, result);
    return result;
}


void UniDefGen::finddefault(const UniConfKey &key, char *p, char *q,
        WvString &result)
{
    if (!p)
    {
        result = UniFilterGen::get(++q);
        if (!result.isnull())
            replacewildcard(key, q, result);
        return;
    }

    // pop the first segment of p to r
    char *r = strchr(p, '/');
    if (r)
        *r++ = '\0';

    // append p to q
    char *s = strchr(q, '\0');
    *s++ = '/';
    *s = 0;
    q = strcat(q, p);

    // try this literal path
    finddefault(key, r, q, result);

    if (!result.isnull())
        return;

    // replace what used to be p with a *
    *s++ = '*';
    *s = '\0';
    finddefault(key, r, q, result);

    if (r)
        *--r = '/';
}


void UniDefGen::replacewildcard(const UniConfKey &key, char *p,
        WvString &result)
{
    // check if the result wants a wildcard ('*n')
    const char *s = result.cstr();
    if (strlen(s) < 2 || s[0] != '*')
        return;

    int idx = atoi(s+1);
    if (idx == 0)
        return;

    // search backwards for segment num of the n'th wildcard
    UniConfKey k(p);
    int loc = key.numsegments();
    for (int i = 0; i < idx; i++)
    {
        if (i != 0)
        {
            k = k.removelast();
            loc--;
        }
        while (!k.last().iswild())
        {
            k = k.removelast();
            loc--;
            if (k.isempty())
            {
                // oops, ran out of segments!
                result = WvString::null;
                return;
            }
        }
    }

    // pull the literal from that segment num of the key
    result = key.segment(loc-1);
}
