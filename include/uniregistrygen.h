/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A generator that exposes the windows registry.
 *
 * When linking statically, use the following #pragma to ensure this 
 * generator gets registered:
 * #pragma comment(linker, "/include:?UniRegistryGenMoniker@@3V?$WvMoniker@VUniConfGen@@@@A")
 */
#ifndef __UNICONFREGISTRY_H
#define __UNICONFREGISTRY_H

#include "uniconfgen.h"
#include "wvlog.h"
#include "windows.h"

/**
 * A generator that exposes the windows registry.
 *
 * To mount, use the moniker "registry:".
 *
 */
class UniRegistryGen : public UniConfGen
{
    WvLog m_log;
    HKEY m_hRoot;

    HKEY follow_path(const UniConfKey &key, bool create, bool *isValue);

public:
    UniRegistryGen(WvString _base);
    virtual ~UniRegistryGen();

    /***** Overridden methods *****/
    
    virtual bool isok();
    virtual WvString get(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual bool exists(const UniConfKey &key);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);
};

#if 0
class UniRegistryGen::Iter : public UniRegistryGen::Iter
{
public:
    /** Destroys the iterator. */
    virtual ~Iter() { }

    /**
     * Rewinds the iterator.
     * Must be called prior to the first invocation of next().
     */
    virtual void rewind() = 0;

    /**
     * Seeks to the next element in the sequence.
     * Returns true if that element exists.
     * Must be called prior to the first invocation of key().
     */
    virtual bool next() = 0;

    /** Returns the current key. */
    virtual UniConfKey key() const = 0;
};
#endif
#endif // __UNICONFREGISTRY_H
