/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 */

/** /file
 * A UniConf data source location abstraction.
 */
#ifndef __UNICONFLOCATION_H
#define __UNICONFLOCATION_H

#include "wvstring.h"

/**
 * Represents a data source location that can be mounted into a
 * UniConf hierarchy.
 * <p>
 * UniConfLocations form a restricted subset of URLs where the
 * protocol identifies the mechanism that will be used to obtain
 * the data, and the path contains protocol-specific information
 * on how to find that data.
 * </p>
 */
class UniConfLocation
{
    WvString _proto;
    WvString _payload;

public:
    UniConfLocation(WvStringParm location);
    UniConfLocation(WvStringParm proto, WvStringParm payload);
    UniConfLocation(const UniConfLocation &other);

    /**
     * Returns the protocol field of the location.
     * @return the protocol field
     */
    inline WvString proto() const
        { return _proto; }

    /**
     * Returns the payload field of the location.
     * @return the payload field
     */
    inline WvString payload() const
        { return _payload; }

    /**
     * Returns the canonical string representation of the location.
     * @return the location as a string
     */
    WvString printable() const;
    inline operator WvString () const
        { return printable(); }
        
    /**
     * Determines if two locations are equal.
     * @param other the other location
     */
    bool operator== (const UniConfLocation &other) const;
    inline bool operator!= (const UniConfLocation &other) const
        { return ! (*this == other); }

    /**
     * Produces a total ordering of locations.
     * @param other the other path
     */
    bool operator< (const UniConfLocation &other) const;
};

#endif // __UNICONFLOCATION_H
