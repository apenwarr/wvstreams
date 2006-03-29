/*  
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A generator to make a UniConf object out of a WvConf.
 */
#include "wvconf.h"
#include "uniwvconfgen.h"
#include "wvmoniker.h"

/**
 * A wrapper class for the wvconf iters to provide a UniConfGen iter.
 */
class UniWvConfGen::WvConfIter : public UniConfGen::Iter
{
protected:
    WvConfigSection::Iter i;

public:
    WvConfIter(WvConfigSection *sect);

    /***** Overridden members *****/

    virtual void rewind();
    virtual bool next();
    virtual UniConfKey key() const;
    virtual WvString value() const;
};


static IUniConfGen *creator(WvStringParm s)
{
    return new UniWvConfGen(new WvConf(s));
}

static WvMoniker<IUniConfGen> reg("wvconf", creator);


void UniWvConfGen::notify(void *userdata, WvStringParm section,
			  WvStringParm entry, WvStringParm oldval,
			  WvStringParm newval)
{
    UniConfKey key(section, entry);

    tempvalue = newval;
    tempkey = &key;
    delta(key, newval);
    tempkey = NULL;
}


UniWvConfGen::UniWvConfGen(WvConf *_cfg):
    tempkey(NULL), tempvalue(), cfg(_cfg)
{
    cfg->add_callback(WvConfCallback(this, &UniWvConfGen::notify), NULL,
		      "", "", this);
}


UniWvConfGen::~UniWvConfGen()
{
    if (cfg)
	delete cfg;
}


WvString UniWvConfGen::get(const UniConfKey &key)
{
    if (tempkey && key == *tempkey)
	return tempvalue;
    else
	return cfg->get(key.first(), key.last(key.numsegments() - 1));
}


void UniWvConfGen::set(const UniConfKey &key, WvStringParm value)
{
    WvString section = key.first();
    WvString keyname = key.last(key.numsegments() - 1);

    WvConfigSection *sect = (*cfg)[section];
    if (value == WvString::null && sect)
        cfg->delete_section(key);
    else
        cfg->set(section, keyname, value);
}


void UniWvConfGen::setv(const UniConfPairList &pairs)
{
    setv_naive(pairs);
}


bool UniWvConfGen::haschildren(const UniConfKey &key)
{
    WvConfigSection *sect = (*cfg)[key];
    if (sect)
        return true;
    return false;
}


UniWvConfGen::Iter *UniWvConfGen::iterator(const UniConfKey &key)
{
    WvConfigSection *sect = (*cfg)[key];

    if (sect)
        return new WvConfIter(sect);
    else
        return NULL;
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


WvString UniWvConfGen::WvConfIter::value() const
{
    return i->value;
}
