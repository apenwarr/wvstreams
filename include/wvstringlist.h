/* -*- Mode: C++ -*-
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

DeclareWvList2(WvStringListBase, WvString);

class WvStringList : public WvStringListBase
{
public:
    WvString join(const char *joinchars = " ") const;
    void split(WvStringParm s, const char *splitchars = " \t\r\n",
	       int limit = 0);
    void splitstrict(WvStringParm s, const char *splitchars = " \t\r\n",
	       int limit = 0);
    void fill(const char * const *array);
    void append(WvStringParm str);
    void append(const WvString *strp, bool autofree, char *id = NULL);
    WvString popstr();
};

#endif // __WVSTRINGLIST_H
