/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen framework to simplify writing filtering generators.
 */
#include "unifiltergen.h"

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
