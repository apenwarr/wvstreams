/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf generator that stores keys in memory.
 */
#include "uniconf.h"
#include "unicachegen.h"
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

    return new UniCacheGen(gen);
}

static WvMoniker<UniConfGen> reg("cache", creator);


/***** UniCacheGen *****/

UniCacheGen::UniCacheGen(UniConfGen *_inner)
    : log("UniCache", WvLog::Debug1), inner(_inner)
{
    loadtree();

    if (inner)
        inner->setcallback(UniConfGenCallback(this,
            &UniCacheGen::deltacallback), NULL);
}


UniCacheGen::~UniCacheGen()
{
    delete inner;
}

void UniCacheGen::loadtree(const UniConfKey &key)
{
    UniConfGen::Iter *i = inner->iterator(key);

    if (!i)
        return;

    for (i->rewind(); i->next();)
    {
        WvString newkey("%s/%s", key, (*i).key());
        WvString value(inner->get(newkey));

        if (!!value)
            UniTempGen::set(newkey, value);

        if(inner->haschildren(newkey))
            loadtree(newkey);
    }

    delete i;
}

void UniCacheGen::deltacallback(const UniConfKey &key, WvStringParm value,
                                void *userdata)
{
    UniTempGen::set(key, value);
}

void UniCacheGen::set(const UniConfKey &key, WvStringParm value)
{
    inner->set(key, value);
}
