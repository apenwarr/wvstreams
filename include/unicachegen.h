/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A UniConf generator that stores keys in memory.
 */
#ifndef __UNICACHEGEN_H
#define __UNICACHEGEN_H

#include "unitempgen.h"
#include "uniconftree.h"
#include "wvlog.h"

/**
 * A UniConf generator that stores keys in memory.
 * 
 * Maintains a dirtyness indicator that is set whenever the contents
 * are changed.  Also dispatches notifications on such changes.
 */
class UniCacheGen : public UniTempGen
{
protected:
    WvLog log;
    UniConfGen *inner;

    void loadtree(const UniConfKey &key = "");
    void deltacallback(const UniConfKey &key, WvStringParm value,
                       void *userdata);

public:
    UniCacheGen(UniConfGen *_inner);
    virtual ~UniCacheGen();

    /***** Overridden members *****/
    virtual void set(const UniConfKey &key, WvStringParm value);
};

#endif // __UNICACHEGEN_H
