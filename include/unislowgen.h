/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen that makes everything slow.
 */
#ifndef __UNISLOWGEN_H
#define __UNISLOWGEN_H

#include "unifiltergen.h"

/**
 * A UniConfGen that makes all "potentially synchronous" operations *really*
 * slow, so you'll know for sure if you're calling UniConf synchronous
 * operations when you shouldn't be.  Hint: wrapping this one in a
 * UniCacheGen is supposed to make your program fast again, so a good test
 * is to try a moniker like this:
 * 
 *   cache:slow:tcp:localhost
 * 
 * ...and see if everything still works without major delays.
 */
class UniSlowGen : public UniFilterGen
{
public:
    UniSlowGen(IUniConfGen *inner);
    virtual ~UniSlowGen();

    virtual void commit();
    virtual bool refresh();
    virtual WvString get(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);
    virtual Iter *recursiveiterator(const UniConfKey &key);
    
    int how_slow() const
        { return slowcount; }

private:
    int slowcount;
    
    void be_slow(WvStringParm what);
    void be_slow(WVSTRING_FORMAT_DECL)
        { be_slow(WvString(WVSTRING_FORMAT_CALL)); }
};

#endif //__UNISLOWGEN_H
