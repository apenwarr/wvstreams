/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A generator to make a UniConf object out of a WvConf.
 */ 

#ifndef __UNICONFWVGEN_H
#define __UNICONFWVGEN_H

#include "uniconfgen.h"

class UniWvConfGen : public UniConfGen
{
private:
    void notify(void *userdata, WvStringParm section, WvStringParm entry,
		WvStringParm oldval, WvStringParm newval);

protected:
    WvConf &cfg;

    class WvConfIter;

public:
    UniWvConfGen(WvConf &_cfg);

    /***** Overridden members *****/

    virtual WvString get(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);
};

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
};

#endif //__UNICONFWVGEN_H
