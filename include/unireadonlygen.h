/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A read only generator wrapper.
 */
#ifndef __UNIREADONLYGEN_H
#define __UNIREADONLYGEN_H

#include "unifiltergen.h"

/**
 * A generator that wraps another generator and makes it read only.
 *
 * To mount, use the moniker "readonly:" followed by the
 * moniker of the generator to wrap.
 *
 */
class UniReadOnlyGen : public UniFilterGen
{
public:
    UniReadOnlyGen(UniConfGen *inner);

    /***** Overridden members *****/

    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool commit(const UniConfKey &key,
        UniConfDepth::Type depth);
};


#endif // __UNICONFREADONLY_H
