/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Some helper functions for WvStringTable.
 */
#include "wvstringtable.h"
#include "strutils.h"


WvString WvStringTable::join(const char *joinchars)
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
    
    return total;
}


void WvStringTable::split(WvStringParm _s, const char *splitchars)
{
    WvString s(_s);
    char *sptr = s.edit(), *eptr, oldc;
    
    while (sptr && *sptr)
    {
	sptr += strspn(sptr, splitchars);
	eptr = sptr + strcspn(sptr, splitchars);
	
	oldc = *eptr;
	*eptr = 0;
	
	WvString *newstr = new WvString(sptr);
	add(newstr, true);
	
	*eptr = oldc;
	sptr = eptr;
    }
}

