/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen framework to simplify writing filtering generators.
 */
#include "unifiltergen.h"

/***** UniFilterGen *****/

UniFilterGen::UniFilterGen(IUniConfGen *inner) 
    : xinner(NULL)
{
    setinner(inner);
}


UniFilterGen::~UniFilterGen()
{
    delete xinner;
}


void UniFilterGen::setinner(IUniConfGen *inner)
{
    if (xinner)
        xinner->setcallback(UniConfGenCallback(), NULL);
    xinner = inner;
    if (xinner)
        xinner->setcallback(UniConfGenCallback(this,
            &UniFilterGen::gencallback), NULL);
}


void UniFilterGen::commit()
{
    xinner->commit();
}


bool UniFilterGen::refresh()
{
    return xinner->refresh();
}


WvString UniFilterGen::get(const UniConfKey &key)
{
    return xinner->get(key);
}


void UniFilterGen::set(const UniConfKey &key, WvStringParm value)
{
    xinner->set(key, value);
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


void UniFilterGen::gencallback(const UniConfKey &key, WvStringParm value,
                               void *userdata)
{
    delta(key, value);
}
