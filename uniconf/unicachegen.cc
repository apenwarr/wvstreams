/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf generator that stores keys in memory.
 */
#include "uniconf.h"
#include "unicachegen.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"

WV_LINK(UniCacheGen);


// if 'obj' is non-NULL and is a UniConfGen, wrap that; otherwise wrap the
// given moniker.
static IUniConfGen *creator(WvStringParm s, IObject *_obj)
{
    return new UniCacheGen(wvcreate<IUniConfGen>(s, _obj));
}

static WvMoniker<IUniConfGen> reg("cache", creator);


/***** UniCacheGen *****/

UniCacheGen::UniCacheGen(IUniConfGen *_inner)
    : log("UniCache", WvLog::Debug1), inner(_inner)
{
    if (inner)
        inner->add_callback(this, UniConfGenCallback(this,
            &UniCacheGen::deltacallback));
    refreshed_once = false;
}


UniCacheGen::~UniCacheGen()
{
    inner->del_callback(this);
    WVRELEASE(inner);
}


bool UniCacheGen::isok()
{
    return inner->isok();
}


bool UniCacheGen::refresh()
{
    if (!refreshed_once)
    {
	bool ret = inner->refresh();
	loadtree();
	refreshed_once = true;
	return ret;
    }
    else
	return false;
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


void UniCacheGen::deltacallback(const UniConfKey &key, WvStringParm value)
{
    UniTempGen::set(key, value);
}

void UniCacheGen::set(const UniConfKey &key, WvStringParm value)
{
    inner->set(key, value);
}

WvString UniCacheGen::get(const UniConfKey &key)
{
    //inner->get(key);
    inner->flush_buffers(); // update all pending notifications
    return UniTempGen::get(key);
}
