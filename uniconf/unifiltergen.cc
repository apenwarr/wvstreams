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


UniConfKey UniFilterGen::keymap(const UniConfKey &key)
{
    // by default, don't rename the key
    return key;
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
    	xinner->prefetch(keymap(key), recursive);
}


WvString UniFilterGen::get(const UniConfKey &key)
{
    if (xinner)
    	return xinner->get(keymap(key));
    else
    	return WvString::null;
}


void UniFilterGen::set(const UniConfKey &key, WvStringParm value)
{
    if (xinner)
    	xinner->set(keymap(key), value);
}


bool UniFilterGen::exists(const UniConfKey &key)
{
    if (xinner)
    	return xinner->exists(keymap(key));
    else
    	return false;
}


bool UniFilterGen::haschildren(const UniConfKey &key)
{
    if (xinner)
    	return xinner->haschildren(keymap(key));
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
    	return xinner->iterator(keymap(key));
    else
    	return NULL;
}


UniConfGen::Iter *UniFilterGen::recursiveiterator(const UniConfKey &key)
{
    if (xinner)
    	return xinner->recursiveiterator(keymap(key));
    else
    	return NULL;
}


void UniFilterGen::gencallback(const UniConfKey &key, WvStringParm value,
                               void *userdata)
{
    delta(keymap(key), value);
}
