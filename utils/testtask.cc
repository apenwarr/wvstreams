/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Worldvisions Computer Technology, Inc.
 *
 * WvTask test program.
 */

#include "wvstring.h"
#include "wvlinklist.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h> // for alloca()
#include <setjmp.h>
#include <unistd.h> // for sleep()
#include <assert.h>

class WvTaskMan;

class WvTask
{
    friend WvTaskMan;
    typedef void TaskFunc(WvTaskMan &man, void *userdata);
    
    static int taskcount, numtasks, numrunning;
    WvString name;
    int tid;
    
    size_t stacksize;
    bool running, recycled;
    
    WvTaskMan &man;
    jmp_buf mystate;	// used for resuming the task
    
    TaskFunc *func;
    void *userdata;
    
    WvTask(WvTaskMan &_man, size_t _stacksize = 64*1024);
public:
    virtual ~WvTask();
    
    void start(const WvString &_name, TaskFunc *_func, void *_userdata);
    bool isrunning() const
        { return running; }
    void recycle();
};


int WvTask::taskcount, WvTask::numtasks, WvTask::numrunning;


class WvTaskMan
{
    friend WvTask;
    DeclareWvList(WvTask);
    WvTaskList free_tasks;
    
    void get_stack(WvTask &task, size_t size);
    void stackmaster();
    void _stackmaster();
    void do_task();
    jmp_buf stackmaster_task;
    
    WvTask *stack_target;
    jmp_buf get_stack_return;
    
    WvTask *current_task;
    jmp_buf toplevel;
    
public:
    
    WvTaskMan();
    virtual ~WvTaskMan();
    
    WvTask *start(const WvString &name,
		  WvTask::TaskFunc *func, void *userdata,
		  size_t stacksize = 64*1024);
    
    // run() and yield() return the 'val' passed to run() when this task
    // was started.
    int run(WvTask &task, int val = 1);
    int yield(int val = 1);
    
    WvTask *whoami() const
        { return current_task; }
};


WvTask::WvTask(WvTaskMan &_man, size_t _stacksize) : man(_man)
{
    stacksize = _stacksize;
    running = false;
    
    tid = ++taskcount;
    numtasks++;
    
    printf("task %d initializing\n", tid);
    man.get_stack(*this, stacksize);
    printf("task %d initialized\n", tid);
}


WvTask::~WvTask()
{
    numtasks--;
    printf("task %d stopping (%d tasks left)\n", tid, numtasks);
    
    if (running)
    {
	printf("WARNING: task %d was running -- bad stuff may happen!\n",
	       tid);
	numrunning--;
    }
}


void WvTask::start(const WvString &_name, TaskFunc *_func, void *_userdata)
{
    assert(!recycled);
    name = _name;
    name.unique();
    printf("task %d (%s) starting\n", tid, (const char *)name);
    func = _func;
    userdata = _userdata;
    running = true;
    numrunning++;
}


void WvTask::recycle()
{
    if (!running && !recycled)
    {
	man.free_tasks.append(this, true);
	recycled = true;
    }
}


WvTaskMan::WvTaskMan()
{
    printf("task manager up\n");
    current_task = NULL;
    
    if (setjmp(get_stack_return) == 0)
    {
	// initial setup - start the stackmaster() task (never returns!)
	stackmaster();
    }
    // if we get here, stackmaster did a longjmp back to us.
}


WvTaskMan::~WvTaskMan()
{
    printf("task manager down\n");
    if (WvTask::numrunning != 0)
	printf("WARNING!  %d tasks still running at WvTaskMan shutdown!\n",
	       WvTask::numrunning);
}


WvTask *WvTaskMan::start(const WvString &name, 
			 WvTask::TaskFunc *func, void *userdata,
			 size_t stacksize)
{
    WvTask *t;
    
    WvTaskList::Iter i(free_tasks);
    for (i.rewind(); i.next(); )
    {
	if (i().stacksize >= stacksize)
	{
	    t = &i();
	    i.link->auto_free = false;
	    i.unlink();
	    t->recycled = false;
	    t->start(name, func, userdata);
	    return t;
	}
    }
    
    // if we get here, no matching task was found.
    t = new WvTask(*this, stacksize);
    t->start(name, func, userdata);
    return t;
}


int WvTaskMan::run(WvTask &task, int val)
{
    if (&task == current_task)
	return val; // that's easy!
    
    printf("WvTaskMan: switching to task #%d (%s)\n",
	   task.tid, (const char *)task.name);
    
    WvTask *old_task = current_task;
    current_task = &task;
    jmp_buf *state;
    
    if (!old_task)
	state = &toplevel; // top-level call (not in an actual task yet)
    else
	state = &old_task->mystate;
    
    int newval = setjmp(*state);
    if (newval == 0)
    {
	// saved the state, now run the task.
	longjmp(task.mystate, val);
    }
    else
    {
	// someone did yield() (if toplevel) or run() on our task; exit
	current_task = old_task;
	return newval;
    }
}


int WvTaskMan::yield(int val)
{
    if (!current_task)
	return 0; // weird...
    
    printf("WvTaskMan: yielding from task #%d (%s)\n",
	   current_task->tid, (const char *)current_task->name);
    
    int newval = setjmp(current_task->mystate);
    if (newval == 0)
    {
	// saved the task state; now yield to the toplevel.
	longjmp(toplevel, val);
    }
    else
    {
	// back via longjmp, because someone called run() again.  Let's go
	// back to our running task...
	return newval;
    }
}


void WvTaskMan::get_stack(WvTask &task, size_t size)
{
    if (setjmp(get_stack_return) == 0)
    {
	// initial setup
	stack_target = &task;
	longjmp(stackmaster_task, size/1024 + (size%1024 > 0));
    }
    else
    {
	// back from stackmaster - the task is now set up.
	return;
    }
}


void WvTaskMan::stackmaster()
{
    // leave lots of room on the "main" stack before doing our magic
    alloca(1024*1024);
    
    _stackmaster();
}


void WvTaskMan::_stackmaster()
{
    int val;
    
    printf("stackmaster 1\n");
    
    for (;;)
    {
	printf("stackmaster 2\n");
	val = setjmp(stackmaster_task);
	if (val == 0)
	{
	    // just did setjmp; save stackmaster's current state (with
	    // all current stack allocations) and go back to get_stack
	    // (or the constructor, if that's what called us)
	    printf("stackmaster 3\n");
	    longjmp(get_stack_return, 1);
	}
	else
	{
	    // set up a stack frame for the task
	    do_task();
	    
	    // allocate the stack area so we never use it again
	    void *x = alloca(val * (size_t)1024);
	    printf("stackmaster (got stack at %p)\n", x);
	}
    }
}


void WvTaskMan::do_task()
{
    WvTask *task = stack_target;
    
    // back here from longjmp; someone wants stack space.
    printf("stackmaster 4\n");
    if (setjmp(task->mystate) == 0)
    {
	// done the setjmp; that means the target task now has
	// a working jmp_buf all set up.  Leave space on the stack
	// for his data, then repeat the loop (so we can return
	// to get_stack(), and allocate more stack for someone later)
	printf("stackmaster 5\n");
	return;
    }
    else
    {
	// someone did a run() on the task, which
	// means they're ready to make it go.  Do it.
	for (;;)
	{
	    printf("stackmaster 6\n");
	    if (task->func && task->running)
	    {
		task->func(*this, task->userdata);
		task->name = "DEAD";
		task->running = false;
		task->numrunning--;
	    }
	    printf("stackmaster 7\n");
	    yield();
	}
    }
}


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
