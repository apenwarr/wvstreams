/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * A read only generator wrapper.
 */
#ifndef __UNICONFREADONLY_H
#define __UNICONFREADONLY_H

#include "uniconfgen.h"

/**
 * A generator that wraps another generator and makes it read only.
 * <p>
 * To mount, use the moniker "readonly://" followed by the
 * moniker of the generator to wrap.
 * </p>
 */
class UniConfReadOnlyGen : public UniConfFilterGen
{
public:
    UniConfReadOnlyGen(UniConfGen *inner);

    /***** Overridden members *****/

    virtual UniConfLocation location() const;
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool commit(const UniConfKey &key, UniConf::Depth depth);
};


/**
 * A factory for UniConfReadOnlyGen instances.
 */
class UniConfReadOnlyGenFactory : public UniConfGenFactory
{
public:
    virtual UniConfReadOnlyGen *newgen(const UniConfLocation &location);
};


#endif // __UNICONFREADONLY_H
