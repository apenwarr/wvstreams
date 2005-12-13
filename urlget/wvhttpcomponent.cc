/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Componentization stuff for wvhttppool.h.  Constitutes cheating.
 */
#include "wvhttppool.h"
#include "wvmoniker.h"
#include "wvistreamlist.h"

static WvHttpPool *pool;


static void pool_init()
{
    // FIXME: we never free it!
    if (!pool)
    {
	pool = new WvHttpPool;
	WvIStreamList::globallist.append(pool, false, "pool_init urlpool");
    }
}


static IWvStream *creator(WvStringParm s)
{
    pool_init();
    return pool->addurl(WvString("http:%s", s), "", false);
}


static IWvStream *screator(WvStringParm s)
{
    pool_init();
    return pool->addurl(WvString("https:%s", s), "", false);
}


static const UUID uuid = {0x2da3d708, 0x037e, 0x4209,
			  {0x8d, 0x8a, 0x70, 0x63, 0xf8, 0x28, 0xd5, 0x43}};
static WvMoniker<IWvStream> reg("http", uuid, creator);

static const UUID uuids = {0x3f04edb7, 0x92a9, 0x4be3,
			   {0xa0, 0xb3, 0xb8, 0x4f, 0x8e, 0x7e, 0x90, 0x5d}};
static WvMoniker<IWvStream> regs("https", uuids, screator);
