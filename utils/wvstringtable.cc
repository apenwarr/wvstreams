/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Some helper functions for WvStringTable.
 */
#include "wvstringtable.h"
#include "strutils.h"


WvString WvStringTable::join(const char *joinchars) const
{
    return ::strcoll_join(*this, joinchars);
}


void WvStringTable::split(WvStringParm s, const char *splitchars,
    int limit)
{
    return ::strcoll_split(*this, s, splitchars, limit);
}

void WvStringTable::splitstrict(WvStringParm s, const char *splitchars,
    int limit)
{
    return ::strcoll_splitstrict(*this, s, splitchars, limit);
}
