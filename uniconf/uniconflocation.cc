/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 */

/** /file
 * A UniConf data source location abstraction.
 */
#include "uniconflocation.h"
#include <string.h>

/***** UniConfLocation *****/

UniConfLocation::UniConfLocation(WvStringParm location)
{
    char *colon = strstr(location.cstr(), "://");
    if (! colon)
    {
        _proto = "unknown";
        _payload = "";
    }
    else
    {
        int pos = colon - location.cstr();
        _proto = location;
        _proto.edit()[pos] = '\0';
        _payload = location.cstr() + pos + 3;
        _payload.unique();
    }
}


UniConfLocation::UniConfLocation(WvStringParm proto,
    WvStringParm payload) :
    _proto(proto), _payload(payload)
{
}


UniConfLocation::UniConfLocation(const UniConfLocation &other) :
    _proto(other._proto), _payload(other._payload)
{
}


WvString UniConfLocation::printable() const
{
    return WvString("%s://%s", _proto, _payload);
}


bool UniConfLocation::operator== (const UniConfLocation &other) const
{
    return _proto == other._proto && _payload == other._payload;
}


bool UniConfLocation::operator< (const UniConfLocation &other) const
{
    int cmp = strcmp(_proto, other._proto);
    return cmp < 0 || cmp == 0 && strcmp(_payload, other._payload) < 0;
}
