/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 *
 * A UniConf generator that caches keys/values in memory.
 */
#ifndef __UNIREPLICATEGEN_H
#define __UNIREPLICATEGEN_H

#include "uniconftree.h"
#include "wvlog.h"

/**
 * A UniConf generator that replicates generators between an ordered list
 * of generators, with the priority given by the list.
 */
class UniReplicateGen : public UniConfGen
{
protected:
    IUniConfGenList gens;

    IUniConfGen *first_ok() const;
    void replicate(const UniConfKey &key = "");
    void deltacallback(const UniConfKey &key, WvStringParm value,
                       void *userdata);

public:
    UniReplicateGen();
    UniReplicateGen(const IUniConfGenList &_gens, bool auto_free = true);
    
    void prepend(IUniConfGen *gen, bool auto_free = true);
    void append(IUniConfGen *gen, bool auto_free = true);

    /***** Overridden members *****/
    virtual bool isok();
    virtual bool refresh();
    virtual void commit();
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual WvString get(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);
};

#endif // __UNIREPLICATEGEN_H
