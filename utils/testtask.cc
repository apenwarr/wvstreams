/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Worldvisions Computer Technology, Inc.
 *
 * WvTask test program.
 */
#include "wvtask.h"
#include <unistd.h> // for sleep()


WvTask *ga, *gb;


void gentask(WvTaskMan &man, void *userdata)
{
    char *str = (char *)userdata;
    int count = 0;
    
    printf("Gentask starting %s\n", str);
    
    while (count < 3)
    {
	printf("%s #%d -- %p\n", str, ++count, &str);
	sleep(1);
	if (count % 2)
	{
	    if (man.whoami() == ga)
	    {
		printf("Doing gb:\n");
		man.run(*gb);
	    }
	    else
	    {
		printf("Doing ga:\n");
		man.run(*ga);
	    }
	}
	
	man.yield();
    }
    
    printf("Gentask ending %s\n", str);
}


int main()
{
    WvTaskMan man;
    
    ga = man.start("atask", gentask, (void *)"a");
    gb = man.start("btask", gentask, (void *)"b");
    
    for (int x = 0; x < 10; x++)
    {
	printf("main1:\n");
	man.run(*ga);
	printf("main2:\n");
	man.run(*gb);
	
	gb->recycle();
	
	if (!gb->isrunning())
	    gb = man.start("bbtask", gentask, (void *)"bb");
	if (!ga->isrunning())
	    ga = man.start("aatask", gentask, (void *)"aa");
    }
    
    return 0;
}
