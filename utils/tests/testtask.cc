/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvTask test program.
 */
#include "wvtask.h"
#include <unistd.h> // for sleep()

WvTask *ga, *gb;

WvTaskMan *gman;

void gentask(void *userdata)
{
    char *str = (char *)userdata;
    int count = 0, delay = 0;
    
    printf("Gentask starting %s\n", str);
    
    while (count < 3)
    {
	printf("%s count %d -- %p\n", str, ++count, &str);
	usleep(delay*1000);
	if (count % 2)
	{
	    if (gman->whoami() == ga)
	    {
		if (gb)
		{
		    printf("Doing gb:\n");
		    gman->run(*gb, 400);
		}
	    }
	    else
	    {
		if (ga)
		{
		    printf("Doing ga:\n");
		    gman->run(*ga, 400);
		}
	    }
	}
	
	delay = gman->yield();
    }
    
    printf("Gentask ending %s\n", str);
}


int main()
{
    WvTaskMan man;
    
    gman = &man;
    ga = man.start("atask", gentask, (void *)"a");
    gb = man.start("btask", gentask, (void *)"b", 8192);
    
    // simple test
    for (int x = 0; x < 10; x++)
    {
	printf("main1:\n");
	man.run(*ga, 400);
	printf("main2:\n");
	man.run(*gb, 400);
	
	if (!gb->isrunning())
	{
	    gb->recycle();
	    gb = man.start("bbtask", gentask, (void *)"bb");
	}
	
	if (!ga->isrunning())
	{
	    ga->recycle();
	    ga = man.start("aatask", gentask, (void *)"aa");
	}
    }
    
    // finish the tasks
    while (ga->isrunning())
	man.run(*ga, 0);
    while (gb->isrunning())
	man.run(*gb, 0);
    
    ga->recycle();
    gb->recycle();
    ga = NULL;
    gb = NULL;
    
    // stress test
    WvTaskList tasks;
    for (int x = 1; x <= 20; x++)
    {
	printf("x == %d\n", x);
	for (int y = 1; y <= 10; y++)
	{
	    WvTask *t = man.start("stresstask", gentask,
				  (void *)"testy",
				  16384);
	    tasks.append(t, false);
	}
	
	WvTaskList::Iter i(tasks);
	for (i.rewind(); i.next(); )
	    man.run(i(), 10);
    }
    while (tasks.count())
    {
	WvTaskList::Iter i(tasks);
	for (i.rewind(); i.next(); )
	{
	    man.run(i(), 100);
	    if (!i().isrunning())
	    {
		i().recycle();
		i.unlink();
		i.rewind();
	    }
	}
    }
    
    return 0;
}
