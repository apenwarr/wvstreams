/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvStrings are used a lot more often than WvStringLists, so the List need
 * not be defined most of the time.  Include this file if you need it.
 *
 */
#ifndef __WVSTRINGLIST_H
#define __WVSTRINGLIST_H

#include "wvstring.h"
#include "wvlinklist.h"

DeclareWvList2(WvString,
	       WvString join(const char *joinchars = " \t") const;
	       void split(const WvString &s, const char *splitchars = " \t");
	       );

#endif // __WVSTRINGLIST_H
