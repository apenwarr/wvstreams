/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test of the WvStreamClone class.  Clones stdin and stdout, prints whatever
 * it receives, and terminates when the stream closes.  (Ctrl-D)
 *
 */

#include "wvstreamclone.h"

int main()
{
    WvStreamClone c(&wvcon);
    
    while (wvcon->isok() && c.isok())
    {
	char *line = NULL;
	if (c.select(-1))
	    line = wvcon->getline(-1);
	if (line)
	    c.print("%s\n", line);
    }
    
    wvcon->print("isok: %s/%s\n", wvcon->isok(), c.isok());
}
