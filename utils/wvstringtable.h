/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * WvStrings are used a lot more often than WvStringTables, so the Table need
 * not be defined most of the time.  Include this file if you need it.
 *
 */
#ifndef __WVSTRINGTABLE_H
#define __WVSTRINGTABLE_H

#include "wvstring.h"
#include "wvhashtable.h"

DeclareWvTable2(WvString,
		WvString join(const char *joinchars = " \t");
                void split(const WvString &s, const char *splitchars = " \t");
		);

#endif // __WVSTRINGTABLE_H
