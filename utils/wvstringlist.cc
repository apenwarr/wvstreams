/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Some helper functions for WvStringList.
 * 
 * This is blatantly block-copied from WvStringTable, but I don't care!  Hah!
 * (I just know I'm going to regret this someday...)
 */
#include "wvstringlist.h"
#include "strutils.h"


WvString WvStringList::join(const char *joinchars) const
{
    WvStringList::Iter s(*this);
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


void WvStringList::split(WvStringParm _s, const char *splitchars,
			 int limit)
{
    WvString s(_s);
    char *sptr = s.edit(), *eptr, oldc;

    while (sptr && *sptr)
    {
	--limit;
	if( limit )
	{
	    sptr += strspn(sptr, splitchars);
	    eptr = sptr + strcspn(sptr, splitchars);
	}
	else
	{
	    sptr += strspn(sptr, splitchars);
	    eptr = sptr + strlen(sptr);
	}
	
	oldc = *eptr;
	*eptr = 0;
	
	WvString *newstr = new WvString(sptr);
	newstr->unique();
	append(newstr, true);
	
	*eptr = oldc;
	sptr = eptr;
    }
}

