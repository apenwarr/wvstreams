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

class WvTaskMan;

class WvTask
{
public:
    typedef void TaskFunc(WvTaskMan &man, void *userdata);
    
    static int taskcount, numtasks;
    int tid;
    
    WvTask(WvTaskMan &man, size_t stacksize = 64*1024);
    virtual ~WvTask();
    
    void start(TaskFunc *func, void *userdata);
    
    WvTaskMan &man;
    jmp_buf mystate;	// used for resuming the task
    
    TaskFunc *func;
    void *userdata;
};

DeclareWvList(WvTask);

int WvTask::taskcount, WvTask::numtasks;


class WvTaskMan
{
public:
    WvTaskMan();
    virtual ~WvTaskMan();
    
    void run(WvTask &task, int val = 1);
    int yield();
    
    void get_stack(WvTask &task, size_t size);
    void stackmaster();
    void _stackmaster();
    void _stackmaster_go();
    jmp_buf stackmaster_task;
    
    WvTask *stack_target;
    jmp_buf get_stack_return;
    
    WvTask *current_task;
    jmp_buf *return_here;
};


WvTask::WvTask(WvTaskMan &_man, size_t stacksize) : man(_man)
{
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
}


void WvTask::start(TaskFunc *_func, void *_userdata)
{
    printf("task %d starting\n", tid);
    func = _func;
    userdata = _userdata;
}


WvTaskMan::WvTaskMan()
{
    printf("task manager up\n");
    current_task = NULL;
    return_here = NULL;
    
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
}


void WvTaskMan::run(WvTask &task, int val)
{
    // save the previous task state
    jmp_buf *old_buf = return_here;
    WvTask *old_task = current_task;
    
    jmp_buf runbuf;
    
    if (current_task)
	return_here = &current_task->mystate;
    else
	return_here = &runbuf;
    current_task = &task;
    
    int newval = setjmp(*return_here);
    if (newval == 0)
    {
	// our state has been saved; run the task.
	longjmp(task.mystate, val);
	
	// (not reached)
    }
    else
    {
	// we're back via the longjmp, so the task has done yield().
	return_here = old_buf;
	current_task = old_task;
	return;
    }
}


int WvTaskMan::yield()
{
    if (!current_task)
	return 0; // weird...
    
    int val = setjmp(current_task->mystate);
    
    if (val == 0)
    {
	// saved the task state; now yield to the caller of run().
	longjmp(*return_here, 1);
    }
    else
    {
	// back via longjmp, because someone called run().  Let's go
	// back to our running task...
	return val;
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
	    _stackmaster_go();
	    
	    // allocate the stack area so we never use it again
	    void *x = alloca(val * (size_t)1024);
	    printf("stackmaster (got stack at %p)\n", x);
	}
    }
}


void WvTaskMan::_stackmaster_go()
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
	    if (task->func)
		task->func(*this, task->userdata);
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
    
    for (;;) 
    {
	printf("%s #%d -- %p\n", str, ++count, &str);
	usleep(400*1000);
	if (count % 2)
	{
	    if (man.current_task == ga)
	    {
		printf("Doing gb\n");
		man.run(*gb);
	    }
	    else
	    {
		printf("Doing ga\n");
		man.run(*ga);
	    }
	}
	
	printf("%s yielding\n", str);
	man.yield();
    }
    
    printf("Gentask ending %s\n", str);
}


int main()
{
    WvTaskMan man;
    WvTask a(man), b(man);
    
    ga = &a;
    gb = &b;
    
    a.start(gentask, (void *)"a");
    b.start(gentask, (void *)"b");
    
    for (int x = 0; x < 50; x++)
    {
	printf("main\n");
	man.run(a);
	man.run(b);
    }
    
    return 0;
}
