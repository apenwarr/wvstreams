/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Create new WvTasks until we crash.
 */
#include "wvtask.h"

WvTaskMan *gman;

void gentask(void *userdata)
{
    static int xc = 0;
    
    int x = ++xc;
    
    printf("  Gentask %d (%p)\n", x, userdata);
    gman->yield();

    while (1)
    {
	printf("  continue Gentask %d (%p)\n", x, userdata);
	if (userdata)
	    gman->run(*(WvTask *)userdata);
	else
	    gman->yield();
    }
    
    printf("  Gentask ending %d\n", x);
}


int main()
{
    WvTaskMan man;
    gman = &man;
    
    WvTask *t = NULL, *last_t = NULL;
    
    // simple test
    for (int x = 0; x < 100; x++)
    {
	printf("starting %d:\n", x);
	last_t = t;
	t = man.start("task", gentask, last_t);
	man.run(*t);
    }
    
    man.run(*t);
    
    return 0;
}
