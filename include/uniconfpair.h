/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConf key-value pair storage abstraction.
 */
#ifndef __UNICONFPAIR_H
#define __UNICONFPAIR_H

#include "uniconfkey.h"
#include "wvstring.h"
#include "wvhashtable.h"


/** Represents a simple key-value pair. */
class UniConfPair
{
    UniConfKey xkey;  /*!< the name of this entry */
    WvString xvalue;  /*!< the value of this entry */

    /** undefined. */
    UniConfPair(const UniConfPair &);

    /** undefined. */
    UniConfPair &operator= (const UniConfPair &);

public:
    /**
     * Creates a UniConfPair.
     * "key" is the key
     * "value" is the value
     */
    UniConfPair(const UniConfKey &key, WvStringParm value);

    /**
     * Returns the key field.
     * "the" is key
     */
    const UniConfKey &key() const
        { return xkey; }

    /**
     * Returns the value field.
     * "the" is value
     */
    const WvString &value()
        { return xvalue; }

    /**
     * Sets the value field.
     * "value" is the new value
     */
    void setvalue(WvStringParm value);
};

DeclareWvDict(UniConfPair, UniConfKey, key());

extern UniConfPairDict null_UniConfPairDict;

#endif //__UNICONFPAIR_H
