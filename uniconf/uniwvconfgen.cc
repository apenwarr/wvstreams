/*  
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A generator to make a UniConf object out of a WvConf.
 */

#include "uniwvconfgen.h"

/***** UniWvConfGen *****/

UniWvConfGen::UniWvConfGen(WvConf &_cfg)
    : cfg(_cfg)
{
}


WvString UniWvConfGen::get(const UniConfKey &key)
{
    fprintf(stderr, "Section: %s, Key: %s", WvString(key.first()).cstr(),
WvString(key.last(key.numsegments() - 1)).cstr());
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
        return new WvConfIter(sect);
    else
        return new UniConfGen::NullIter();
}



/***** UniWvConfGen::WvConfIter *****/

UniWvConfGen::WvConfIter::WvConfIter(WvConfigSection *sect)
    : i(*sect)
{
}


void UniWvConfGen::WvConfIter::rewind()
{
    i.rewind();
}


bool UniWvConfGen::WvConfIter::next()
{
    return i.next();
}


UniConfKey UniWvConfGen::WvConfIter::key() const
{
    return i->name;
}
