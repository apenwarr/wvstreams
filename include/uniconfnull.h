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
 *
 * To mount, use the moniker "null:".
 *
 */
class UniConfNullGen : public UniConfGen
{
public:
    UniConfNullGen();
    virtual ~UniConfNullGen();

    /***** Overridden methods *****/

    virtual WvString get(const UniConfKey &key);
    virtual bool set(const UniConfKey &key, WvStringParm value);
    virtual bool zap(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);
};


#endif // __UNICONFNULL_H
