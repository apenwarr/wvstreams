/*  
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A generator to make a UniConf object out of a WvConf.
 */

#include "uniconfwvgen.h"
// #include "wvlog.h"
// WvLog log("Hmm?");

WvString UniConfWvGen::get(const UniConfKey &key)
{
    return cfg.get(key.first(), key.last(key.numsegments() - 1));
}

bool UniConfWvGen::set(const UniConfKey &key, WvStringParm value)
{
    cfg.set(key.first(), key.last(key.numsegments() - 1), value);
    if (cfg.get(key.first(), key.last(key.numsegments() - 1)) == value)
        return true;
    return false;
}

bool UniConfWvGen::zap(const UniConfKey &key)
{
    cfg.delete_section(key);

    WvConfigSection *sect = cfg[key];
    if (sect)
        return false;
    return true;
}

bool UniConfWvGen::haschildren(const UniConfKey &key)
{
    WvConfigSection *sect = cfg[key];
    if (sect)
        return true;
    return false;
}

UniConfWvGen::Iter *UniConfWvGen::iterator(const UniConfKey &key)
{
    WvConfigSection *sect = cfg[key];

    if (sect)
        return new WvConfIter(new WvConfigSection::Iter(*sect));
    else
        return new UniConfGen::NullIter();
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
