/*
 * FIXME: this is much less extensive than tasktest.cc...
 */
#include "wvtest.h"
#include "wvtask.h"

int glob;

static void gentask(void *userdata)
{
    int startval = (int)userdata, val = startval;
    
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
    WVPASS(glob == 1001);
    taskman->run(*b);
    taskman->run(*a);
    WVPASS(glob == 1002);
    taskman->run(*b);
    taskman->run(*b);
    WVPASS(glob == 2003);
    WVPASS(b->isrunning());
    WVPASS(a->isrunning());
    taskman->run(*b);
    WVPASS(glob == 2003);
    WVPASS(!b->isrunning());
    taskman->run(*a);
    taskman->run(*a);
    WVPASS(glob == 1003);
    WVPASS(!a->isrunning());
    a->recycle();
    b->recycle();
    
    taskman->unlink();
}


WVTEST_MAIN()
{
    testme();
    WVPASS("--REPEATING TEST--");
    testme(); // make sure deletion/creation works
}
