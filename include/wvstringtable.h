/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * WvStrings are used a lot more often than WvStringTables, so the Table need
 * not be defined most of the time.  Include this file if you need it.
 *
 */
#ifndef __WVSTRINGTABLE_H
#define __WVSTRINGTABLE_H

#include "wvstring.h"
#include "wvhashtable.h"

DeclareWvTable2(WvStringTableBase, WvString);

class WvStringTable : public WvStringTableBase
{
public:
    WvStringTable(unsigned _numslots) : WvStringTableBase(_numslots) {};
    WvString join(const char *joinchars = " \t") const;
    void split(WvStringParm s, const char *splitchars = " \t",
        int limit = 0);
};

#endif // __WVSTRINGTABLE_H
