/*  
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A generator to make a UniConf object out of a WvConf.
 */ 

#ifndef __UNICONFWVGEN_H
#define __UNICONFWVGEN_H

#include "uniconfgen.h"
#include "wvconf.h"
#include "uniconfiter.h"

class UniWvConfGen : public UniConfGen
{
protected:
    WvConf &cfg;

public:
    UniWvConfGen(WvConf &_cfg) : cfg(_cfg) { }

    typedef UniConfAbstractIter Iter;

    virtual WvString get(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);

    /**
    * WvConfIter
    *
    * A wrapper class for the wvconf iters to provide a uniconf iter.
    */
    class WvConfIter : public Iter
    {
    protected:
        WvConfigSection::Iter *i;

    public:
        WvConfIter(WvConfigSection::Iter *_i) : i(_i) { }
        virtual ~WvConfIter();

        virtual Iter *clone() const;
        virtual void rewind();
        virtual bool next();
        virtual UniConfKey key() const;
    };
};

#endif //__UNICONFWVGEN_H
