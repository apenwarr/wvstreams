/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A hash table container backed by a gdbm database.  See wvgdbmhash.h.
 */
#include "wvgdbmhash.h"
#include "wvstring.h"
#include <time.h>

typedef WvGdbmHash<WvString,WvString> StrStrMap;
typedef WvGdbmHash<WvString,time_t> StrTimeMap;

int main()
{
    StrStrMap ss("/tmp/dbmfile");
    StrTimeMap st("/tmp/dbmfile2");
    
    printf("\nList of strings:\n");
    {
	StrStrMap::Iter i(ss);
	for (i.rewind(); i.next(); )
	{
	    printf("'%s': '%s'\n", i.key().cstr(), i().cstr());
	}
    }
    
    printf("\nList of times:\n");
    {
	StrTimeMap::Iter i(st);
	for (i.rewind(); i.next(); )
	{
	    printf("'%s': '%ld'\n", i.key().cstr(), *i());
	}
    }
    
    ss.add("hello", "world", true);
    ss.add("hello", "world2", true);
    ss.add("yellow", "5", true);
    if (ss.exists("appendable"))
	ss.add("appendable",
	       ss["appendable"].append("-yak"), true);
    else
	ss.add("appendable", "init");
    
    st.add("hello", 65, true);
    st.add("hello", 66, true);
    st.add("mellow", 97650, true);
    if (st.exists("addable"))
	st.add("addable", *st["addable"] + 5, true);
    else
	st.add("addable", 7);
}
