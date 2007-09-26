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
    IUniConfGen *gen = xinner;
    setinner(NULL);
    WVRELEASE(gen);
}


void UniFilterGen::setinner(IUniConfGen *inner)
{
    if (xinner)
	xinner->del_callback(this);
    xinner = inner;
    if (xinner)
        xinner->add_callback(this, wv::bind(&UniFilterGen::gencallback, this,
					    wv::_1, wv::_2));
}


bool UniFilterGen::keymap(const UniConfKey &unmapped_key, UniConfKey &mapped_key)
{
    // by default, don't rename the key
    mapped_key = unmapped_key;
    return true;
}

bool UniFilterGen::reversekeymap(const UniConfKey &mapped_key, UniConfKey &unmapped_key)
{
    // by default, don't rename the key
    unmapped_key = mapped_key;
    return true;
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
    UniConfKey mapped_key;
    if (xinner && keymap(key, mapped_key))
        xinner->prefetch(mapped_key, recursive);
}


WvString UniFilterGen::get(const UniConfKey &key)
{
    UniConfKey mapped_key;
    if (xinner && keymap(key, mapped_key))
    	return xinner->get(mapped_key);
    else
    	return WvString::null;
}


void UniFilterGen::flush_buffers()
{
    if (xinner)
    	xinner->flush_buffers();
}


void UniFilterGen::set(const UniConfKey &key, WvStringParm value)
{
    UniConfKey mapped_key;
    if (xinner && keymap(key, mapped_key))
    	xinner->set(mapped_key, value);
}


void UniFilterGen::setv(const UniConfPairList &pairs)
{
    if (xinner)
	xinner->setv(pairs);
}


bool UniFilterGen::exists(const UniConfKey &key)
{
    UniConfKey mapped_key;
    if (xinner && keymap(key, mapped_key))
    	return xinner->exists(mapped_key);
    else
    	return false;
}


bool UniFilterGen::haschildren(const UniConfKey &key)
{
    UniConfKey mapped_key;
    if (xinner && keymap(key, mapped_key))
    	return xinner->haschildren(mapped_key);
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
    UniConfKey mapped_key;
    if (xinner && keymap(key, mapped_key))
    	return xinner->iterator(mapped_key);
    else
    	return NULL;
}


UniConfGen::Iter *UniFilterGen::recursiveiterator(const UniConfKey &key)
{
    UniConfKey mapped_key;
    if (xinner && keymap(key, mapped_key))
    	return xinner->recursiveiterator(mapped_key);
    else
    	return NULL;
}


void UniFilterGen::gencallback(const UniConfKey &key, WvStringParm value)
{
    UniConfKey unmapped_key;
    if (xinner && reversekeymap(key, unmapped_key))
        delta(unmapped_key, value);
}
