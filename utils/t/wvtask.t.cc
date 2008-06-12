/*
 * FIXME: this is much less extensive than tasktest.cc...
 */
#include "wvtest.h"
#include "wvtask.h"
#ifdef _WIN32
#define usleep(t) Sleep((t) / 1000 + 1) // not exactly right but it will do
#else
#include <unistd.h> // for sleep()
#endif

// BEGIN simple definition
long glob;

static void gentask(void *userdata)
{
    long startval = (long)userdata, val = startval;
    
    while (val - startval < 3)
    {
	glob = ++val;
	WvTaskMan::yield();
    }
}


void testme()
{
    WvTaskMan *taskman = WvTaskMan::get();
    WVPASS(taskman);
    
    WvTask *a = taskman->start("task-a", gentask, (void *)1000);
    WVPASS(a);
    WVPASS(a->isrunning());
    WvTask *b = taskman->start("task-b", gentask, (void *)2000);
    WVPASS(b);
    WVPASS(b->isrunning());
    
    taskman->run(*a);
    WVPASSEQ(glob, 1001);
    WVPASS(a->isrunning());
    WVPASS(b->isrunning());
    taskman->run(*b);
    taskman->run(*a);
    WVPASSEQ(glob, 1002);
    WVPASS(a->isrunning());
    WVPASS(b->isrunning());
    taskman->run(*b);
    taskman->run(*b);
    WVPASSEQ(glob, 2003);
    WVPASS(a->isrunning());
    WVPASS(b->isrunning());
    taskman->run(*b);
    WVPASSEQ(glob, 2003);
    WVPASS(a->isrunning());
    WVPASS(!b->isrunning());
    taskman->run(*a);
    taskman->run(*a);
    WVPASSEQ(glob, 1003);
    WVPASS(!a->isrunning());
    WVPASS(!b->isrunning());
    a->recycle();
    b->recycle();
    
    taskman->unlink();
}
// END simple definition
 
// BEGIN tasktest.cc definition
WvTask *ga, *gb;

WvTaskMan *gman;

void gentask2(void *userdata)
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
// END tasktest.cc definition

WVTEST_MAIN("simple")
{
    testme();
    WVPASS("--REPEATING TEST--");
    testme(); // make sure deletion/creation works
}
#ifdef TASKTEST_IS_CONVERTED
WVTEST_MAIN("tasktest.cc")
{

    
    // simple test
    {
        WvTaskMan *man = WvTaskMan::get();
        gman = man;
        ga = man->start("atask", gentask2, (void *)"a");
        gb = man->start("btask", gentask2, (void *)"b");
        
        for (int x = 0; x < 10; x++)
        {
	    printf("main1:\n");
            man->run(*ga, 400);
            printf("main2:\n");
            man->run(*gb, 400);
	
            // it's still running; can't recycle it yet!
	    //gb->recycle();
	
            if (!gb->isrunning())
                gb = man->start("bbtask", gentask2, (void *)"bb");
            if (!ga->isrunning())
                ga = man->start("aatask", gentask2, (void *)"aa");
        }
    
        // finish the tasks
        while (ga->isrunning())
	    man->run(*ga, 0);
        while (gb->isrunning())
            man->run(*gb, 0);
    
        ga->recycle();
        gb->recycle();
        ga = NULL;
        gb = NULL;
    }
    
    // stress test
    {
        WvTaskMan *man = WvTaskMan::get();
        gman = man;
        WvTaskList tasks;
        
        for (int x = 1; x <= 20; x++)
        {
            printf("x == %d\n", x);
            for (int y = 1; y <= 10; y++)
            {
	        WvTask *t = man->start("stresstask", gentask2, 
                        (void *)"testy", 16384);
                tasks.append(t, false);
            }
	
            WvTaskList::Iter i(tasks);
            for (i.rewind(); i.next(); )
	        man->run(i(), 10);
        }
        while (tasks.count())
        {
            WvTaskList::Iter i(tasks);
            for (i.rewind(); i.next(); )
            {
	        man->run(i(), 100);
                if (!i().isrunning())
	        {
		    i().recycle();
                    i.unlink();
                    i.rewind();
	        }
	    }
        }
        man->unlink();
    }
}
#endif
