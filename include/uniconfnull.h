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
};


/**
 * A factory for UniConfNullGen instances.
 */
class UniConfNullGenFactory : public UniConfGenFactory
{
public:
    virtual UniConfGen *newgen(const UniConfLocation &location,
        UniConf *top);
};


#endif // __UNICONFNULL_H
