/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConf key management abstraction.
 */
#include "uniconfgen.h"
#include "uniconfiter.h"


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



/***** UniFilterGen *****/

UniFilterGen::UniFilterGen(UniConfGen *inner) :
    xinner(NULL)
{
    setinner(inner);
}


UniFilterGen::~UniFilterGen()
{
    delete xinner;
}


void UniFilterGen::setinner(UniConfGen *inner)
{
    if (xinner)
        xinner->setcallback(NULL, NULL);
    xinner = inner;
    if (xinner)
        xinner->setcallback(wvcallback(UniConfGenCallback, *this,
            UniFilterGen::gencallback), NULL);
}


bool UniFilterGen::commit(const UniConfKey &key, UniConfDepth::Type depth)
{
    return xinner->commit(key, depth);
}


bool UniFilterGen::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    return xinner->refresh(key, depth);
}


WvString UniFilterGen::get(const UniConfKey &key)
{
    return xinner->get(key);
}


bool UniFilterGen::set(const UniConfKey &key, WvStringParm value)
{
    return xinner->set(key, value);
}


bool UniFilterGen::zap(const UniConfKey &key)
{
    return xinner->zap(key);
}


bool UniFilterGen::exists(const UniConfKey &key)
{
    return xinner->exists(key);
}


bool UniFilterGen::haschildren(const UniConfKey &key)
{
    return xinner->haschildren(key);
}


bool UniFilterGen::isok()
{
    return xinner->isok();
}


UniConfGen::Iter *UniFilterGen::iterator(const UniConfKey &key)
{
    return xinner->iterator(key);
}


void UniFilterGen::gencallback(UniConfGen *gen,
    const UniConfKey &key, void *userdata)
{
    delta(key);
}
