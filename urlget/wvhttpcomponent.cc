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


static WvMoniker<IWvStream> reg("http", creator);
static WvMoniker<IWvStream> regs("https", screator);
