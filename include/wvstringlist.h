/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * WvStrings are used a lot more often than WvStringLists, so the List need
 * not be defined most of the time.  Include this file if you need it.
 *
 */
#ifndef __WVSTRINGLIST_H
#define __WVSTRINGLIST_H

#include "wvstring.h"
#include "wvlinklist.h"

DeclareWvList2(WvStringListBase, WvString);

class WvStringList : public WvStringListBase
{
public:
    WvString join(const char *joinchars = " \t") const;
    void split(WvStringParm s, const char *splitchars = " \t",
        int limit = 0);
    void fill(const char * const *array);
    WvString popstr();
};

#endif // __WVSTRINGLIST_H
