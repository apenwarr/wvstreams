/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf key management abstraction.
 */
#include "uniconfgen.h"

/***** UniConfGen *****/

UniConfGen::UniConfGen() :
    cb(NULL), cbdata(NULL), hold_nesting(0)
{
}


UniConfGen::~UniConfGen()
{
}


void UniConfGen::hold_delta()
{
    hold_nesting += 1;
}


void UniConfGen::unhold_delta()
{
    assert(hold_nesting > 0);
    if (hold_nesting == 1)
        flush_delta();
    hold_nesting -= 1;
}


void UniConfGen::clear_delta()
{
    deltas.zap();
}


void UniConfGen::flush_delta()
{
    UniConfPairList::Iter it(deltas);
    for (;;)
    {
        it.rewind();
        if (! it.next())
            break;

        UniConfKey key((*it).key());
        WvString value((*it).value());

        it.xunlink();
        dispatch_delta(key, value);
    }
}


void UniConfGen::dispatch_delta(const UniConfKey &key, WvStringParm value)
{
    if (cb)
        cb(key, value, cbdata);
}


void UniConfGen::delta(const UniConfKey &key, WvStringParm value)
{
    if (hold_nesting == 0)
    {
        // not nested, dispatch immediately
        dispatch_delta(key, value);
    }
    else
    {
        hold_delta();
        deltas.add(new UniConfPair(key, value), true);
        unhold_delta();
    }
}


bool UniConfGen::haschildren(const UniConfKey &key)
{
    hold_delta();
    
    Iter *it = iterator(key);
    it->rewind();
    bool children = it->next();
    delete it;
    
    unhold_delta();
    return children;
}


bool UniConfGen::exists(const UniConfKey &key)
{
    return ! get(key).isnull();
}


bool UniConfGen::isok()
{
    return true;
}


void UniConfGen::setcallback(const UniConfGenCallback &callback,
    void *userdata)
{
    cb = callback;
    cbdata = userdata;
}
