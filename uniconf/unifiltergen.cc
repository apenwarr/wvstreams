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
    if (xinner)
    	RELEASE(xinner);
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
    if (xinner)
    	xinner->commit();
}


bool UniFilterGen::refresh()
{
    if (xinner)
    	return xinner->refresh();
    else
    	return false;
}


void UniFilterGen::prefetch(const UniConfKey &key, bool recursive)
{
    if (xinner)
    	xinner->prefetch(key, recursive);
}


WvString UniFilterGen::get(const UniConfKey &key)
{
    if (xinner)
    	return xinner->get(key);
    else
    	return WvString::null;
}


void UniFilterGen::set(const UniConfKey &key, WvStringParm value)
{
    if (xinner)
    	xinner->set(key, value);
}


bool UniFilterGen::exists(const UniConfKey &key)
{
    if (xinner)
    	return xinner->exists(key);
    else
    	return false;
}


bool UniFilterGen::haschildren(const UniConfKey &key)
{
    if (xinner)
    	return xinner->haschildren(key);
    else
    	return false;
}


bool UniFilterGen::isok()
{
    if (xinner)
    	return xinner->isok();
    else
    	return false;
}


UniConfGen::Iter *UniFilterGen::iterator(const UniConfKey &key)
{
    if (xinner)
    	return xinner->iterator(key);
    else
    	return NULL;
}


UniConfGen::Iter *UniFilterGen::recursiveiterator(const UniConfKey &key)
{
    if (xinner)
    	return xinner->recursiveiterator(key);
    else
    	return NULL;
}


void UniFilterGen::gencallback(const UniConfKey &key, WvStringParm value,
                               void *userdata)
{
    delta(key, value);
}
