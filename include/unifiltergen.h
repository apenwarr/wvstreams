/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen framework to simplify writing filtering generators.
 */
#ifndef __UNIFILTERGEN_H
#define __UNIFILTERGEN_H

#include "uniconfgen.h"

/**
 * A UniConfGen that delegates all requests to an inner generator.  If you
 * derive from this, you can selectively override particular behaviours
 * of a sub-generator.
 */
class UniFilterGen : public UniConfGen
{
    UniConfGen *xinner;

protected:
    UniFilterGen(UniConfGen *inner);
    virtual ~UniFilterGen();

    /**
     * Rebinds the inner generator and prepares its callback.
     * The previous generator is NOT destroyed.
     * "inner" must not be null.
     */
    void setinner(UniConfGen *inner);

public:
    /**
     * Returns the inner generator.
     */
    UniConfGen *inner() const
        { return xinner; }

    /***** Overridden methods *****/

    virtual void commit();
    virtual bool refresh();
    virtual WvString get(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual bool isok();
    virtual Iter *iterator(const UniConfKey &key);

protected:
    /**
     * Called by inner generator when a key changes.
     * The default implementation calls delta(key).
     */
    virtual void gencallback(const UniConfKey &key, WvStringParm value,
                             void *userdata);
};

#endif //__UNIFILTERGEN_H
