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
    UniConfKeyList::Iter it(deltas);
    for (;;)
    {
        it.rewind();
        if (! it.next())
            break;
        UniConfKey key(*it);
        it.xunlink();
        dispatch_delta(key);
    }
}


void UniConfGen::dispatch_delta(const UniConfKey &key)
{
    if (cb)
        cb(this, key, cbdata);
}


void UniConfGen::delta(const UniConfKey &key)
{
    if (hold_nesting == 0)
    {
        // not nested, dispatch immediately
        dispatch_delta(key);
    }
    else
    {
        hold_delta();
        deltas.add(new UniConfKey(key), true);
        unhold_delta();
    }
}


bool UniConfGen::commit(const UniConfKey &key, UniConfDepth::Type depth)
{
    return true;
}


bool UniConfGen::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    return true;
}


bool UniConfGen::zap(const UniConfKey &key)
{
    hold_delta();
    
    bool success = true;
    Iter *it = iterator(key);
    for (it->rewind(); it->next();)
    {
        if (! set(it->key(), WvString::null))
            success = false;
    }
    delete it;
    
    unhold_delta();
    return success;
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
