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
static IUniConfGen *creator(WvStringParm s, IObject *obj, void *)
{
    IUniConfGen *gen = NULL;

    if (obj)
        gen = mutate<IUniConfGen>(obj);
    if (!gen)
        gen = wvcreate<IUniConfGen>(s);

    return new UniCacheGen(gen);
}

static WvMoniker<IUniConfGen> reg("cache", creator);


/***** UniCacheGen *****/

UniCacheGen::UniCacheGen(IUniConfGen *_inner)
    : log("UniCache", WvLog::Debug1), inner(_inner)
{
    if (inner)
        inner->setcallback(UniConfGenCallback(this,
            &UniCacheGen::deltacallback), NULL);
}


UniCacheGen::~UniCacheGen()
{
    WVRELEASE(inner);
}


bool UniCacheGen::isok()
{
    return inner->isok();
}


bool UniCacheGen::refresh()
{
    bool ret = inner->refresh();
    loadtree();
    return ret;
}


void UniCacheGen::commit()
{
    inner->commit();
}


void UniCacheGen::loadtree(const UniConfKey &key)
{
    UniConfGen::Iter *i = inner->recursiveiterator(key);
    if (!i) return;

    //assert(false);
    for (i->rewind(); i->next(); )
    {
        WvString value(i->value());
	
	//fprintf(stderr, "Key: '%s'\n", i->key().cstr());
	//fprintf(stderr, "  Val: '%s'\n", value.cstr());

        if (!!value)
            UniTempGen::set(i->key(), value);
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
