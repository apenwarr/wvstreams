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
class UniConfReadOnlyGen : public UniConfGen
{
    UniConfGen *inner;

public:
    UniConfReadOnlyGen(const UniConfLocation &location,
        UniConfGen *inner);
    virtual ~UniConfReadOnlyGen();

    virtual UniConf *make_tree(UniConf *parent, const UniConfKey &key)
        { return inner->make_tree(parent, key); }
    virtual void enumerate_subtrees(UniConf *conf, bool recursive)
        { inner->enumerate_subtrees(conf, recursive); }
    virtual void update(UniConf *&h)
        { inner->update(h); }
    virtual void pre_get(UniConf *&h)
        { inner->pre_get(h); }
    virtual bool isok()
        { return inner->isok(); }
    virtual void update_all()
        { inner->update_all(); }
    virtual void load()
        { inner->load(); }
    virtual void save()
        { }
};


/**
 * A factory for UniConfReadOnlyGen instances.
 */
class UniConfReadOnlyGenFactory : public UniConfGenFactory
{
public:
    virtual UniConfGen *newgen(const UniConfLocation &location,
        UniConf *top);
};


#endif // __UNICONFREADONLY_H
