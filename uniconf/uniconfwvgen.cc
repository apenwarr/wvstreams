/*  
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A generator to make a UniConf object out of a WvConf.
 */

#include "uniconfwvgen.h"

WvString UniConfWvGen::get(const UniConfKey &key)
{
    return cfg.get(key.first(), key.last(key.numsegments()));
}

bool UniConfWvGen::set(const UniConfKey &key, WvStringParm value)
{
    cfg.set(key.first(), key.last(key.numsegments()), value);
    return true;
}

bool UniConfWvGen::zap(const UniConfKey &key)
{
    if (key.first() == key)
        cfg.delete_section(key);
    else 
        remove(key);
    return true;
}

UniConfWvGen::Iter *UniConfWvGen::iterator(const UniConfKey &key)
{
    WvConfigSection *sect = cfg[key];
    return new WvConfIter(new WvConfigSection::Iter(*sect));
}


// WvConfIter

UniConfWvGen::WvConfIter::~WvConfIter()
{
    delete i;
}

UniConfWvGen::Iter *UniConfWvGen::WvConfIter::clone() const
{
    return new WvConfIter(i);
}

void UniConfWvGen::WvConfIter::rewind()
{
    i->rewind();
}

bool UniConfWvGen::WvConfIter::next()
{
    return i->next();
}

UniConfKey UniConfWvGen::WvConfIter::key() const
{
    return (*i)->name;
}
