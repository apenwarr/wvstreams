/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * Some helper functions for WvStringTable.
 */
#include "wvstringtable.h"
#include "strutils.h"


WvString WvStringTable::join(const char *joinchars = " ")
{
    WvStringTable::Iter s(*this);
    size_t totlen;
    WvString total;
    char *te;
    int x;
    
    totlen = 1;
    for (s.rewind(); s.next(); )
	totlen += strlen(s()) + strlen(joinchars);
    
    total.setsize(totlen);
    te = total.edit();
    
    te[0] = 0;
    x = 0;
    for (s.rewind(); s.next(); )
    {
	if (x++)
	    strcat(te, joinchars);
	strcat(te, s());
    }
    
    if (te[0])
	trim_string(te);
    
    return total;
}
