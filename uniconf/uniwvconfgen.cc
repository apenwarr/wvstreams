/*  
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A generator to make a UniConf object out of a WvConf.
 */
#ifdef USE_WVCONFEMU
# include "wvconfemu.h"
#else
# include "wvconf.h"
#endif
#include "uniwvconfgen.h"
#include "wvmoniker.h"


static UniConfGen *creator(WvStringParm s, IObject *, void *obj)
{
#ifdef USE_WVCONFEMU
    assert(obj && "uniwvconfgen compiled with WvConfEmu");
#else
    if (!obj)
	obj = new WvConf(s);
#endif
    
    // FIXME EEK!  This never deletes the WvConf object!
    return new UniWvConfGen(*(WvConf *)obj);
}

static WvMoniker<UniConfGen> reg("wvconf", creator);


void UniWvConfGen::notify(void *userdata, WvStringParm section,
			  WvStringParm entry, WvStringParm oldval,
			  WvStringParm newval)
{
    delta(UniConfKey(section, entry), newval);
}


UniWvConfGen::UniWvConfGen(WvConf &_cfg):
    cfg(_cfg)
{
    cfg.add_callback(WvConfCallback(this, &UniWvConfGen::notify), NULL,
		     "", "", this);
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
