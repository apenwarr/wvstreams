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
