/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * A generator that is always empty and rejects changes.
 */
#ifndef __UNICONFNULL_H
#define __UNICONFNULL_H

#include "uniconfgen.h"

/**
 * A generator that is always empty and rejects changes.
 * <p>
 * To mount, use the moniker "null://".
 * </p>
 */
class UniConfNullGen : public UniConfGen
{
public:
    UniConfNullGen();
    virtual ~UniConfNullGen();

    /***** Overridden methods *****/

    virtual UniConfLocation location() const;
    virtual WvString get(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);
};


/**
 * A factory for UniConfNullGen instances.
 */
class UniConfNullGenFactory : public UniConfGenFactory
{
public:
    virtual UniConfNullGen *newgen(const UniConfLocation &location);
};


#endif // __UNICONFNULL_H
