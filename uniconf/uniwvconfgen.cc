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
    return cfg.get(key.first(), key.last(key.numsegments() - 1));
}


void UniWvConfGen::set(const UniConfKey &key, WvStringParm value)
{
    WvString section = key.first();
    WvString keyname = key.last(key.numsegments() - 1);

    WvConfigSection *sect = cfg[section];
    if (value == WvString::null && sect)
        cfg.delete_section(key);
    else
        cfg.set(section, keyname, value);
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
