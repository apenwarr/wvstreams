/*  
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A generator to make a UniConf object out of a WvConf.
 */

#include "uniwvconfgen.h"
// #include "wvlog.h"
// WvLog log("Hmm?");

WvString UniWvConfGen::get(const UniConfKey &key)
{
    return cfg.get(key.first(), key.last(key.numsegments() - 1));
}

bool UniWvConfGen::set(const UniConfKey &key, WvStringParm value)
{
    cfg.set(key.first(), key.last(key.numsegments() - 1), value);
    if (cfg.get(key.first(), key.last(key.numsegments() - 1)) == value)
        return true;
    return false;
}

bool UniWvConfGen::zap(const UniConfKey &key)
{
    cfg.delete_section(key);

    WvConfigSection *sect = cfg[key];
    if (sect)
        return false;
    return true;
}

bool UniWvConfGen::haschildren(const UniConfKey &key)
{
    WvConfigSection *sect = cfg[key];
    if (sect)
        return true;
    return false;
}

UniWvConfGen::Iter *UniWvConfGen::iterator(const UniConfKey &key)
{
    WvConfigSection *sect = cfg[key];

    if (sect)
        return new WvConfIter(new WvConfigSection::Iter(*sect));
    else
        return new UniConfGen::NullIter();
}


// WvConfIter

UniWvConfGen::WvConfIter::~WvConfIter()
{
    delete i;
}

UniWvConfGen::Iter *UniWvConfGen::WvConfIter::clone() const
{
    return new WvConfIter(i);
}

void UniWvConfGen::WvConfIter::rewind()
{
    i->rewind();
}

bool UniWvConfGen::WvConfIter::next()
{
    return i->next();
}

UniConfKey UniWvConfGen::WvConfIter::key() const
{
    return (*i)->name;
}
