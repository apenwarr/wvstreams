/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf key management abstraction.
 */
#include "uniconfgen.h"

/***** UniConfGen *****/

UniConfGen::UniConfGen() :
    cb(NULL), cbdata(NULL)
{
}


UniConfGen::~UniConfGen()
{
}


void UniConfGen::delta(const UniConfKey &key)
{
    if (cb)
        cb(this, key, cbdata);
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
    bool success = true;
    
    Iter *it = iterator(key);
    for (it->rewind(); it->next();)
    {
        if (! set(it->key(), WvString::null))
            success = false;
    }
    delete it;
    return success;
}


bool UniConfGen::haschildren(const UniConfKey &key)
{
    Iter *it = iterator(key);
    it->rewind();
    bool children = it->next();
    delete it;
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
