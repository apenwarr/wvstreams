/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A set of classes that provide co-operative multitasking support.  See
 * wvtask.h for more information.
 */
#include "wvtask.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <malloc.h> // for alloca()
#if 0
// FIXME: apparently, non-Linux platforms needs this (for alloca?).
#include <stdlib.h>
#endif

int WvTask::taskcount = 0;
int WvTask::numtasks = 0;
int WvTask::numrunning = 0;


WvTask::WvTask(WvTaskMan &_man, size_t _stacksize) : man(_man)
{
    stacksize = _stacksize;
    running = recycled = false;
    func = NULL;
    userdata = NULL;
    
    tid = ++taskcount;
    numtasks++;
    magic_number = WVTASK_MAGIC;
    
    man.get_stack(*this, stacksize);
}


WvTask::~WvTask()
{
    numtasks--;
    
    if (running)
    {
	numrunning--;
    }
    
    magic_number = 42;
}


void WvTask::start(WvStringParm _name, TaskFunc *_func, void *_userdata)
{
    assert(!recycled);
    name = _name;
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
    stack_target = NULL;
    current_task = NULL;
    magic_number = -WVTASK_MAGIC;
    
    if (setjmp(get_stack_return) == 0)
    {
	// initial setup - start the stackmaster() task (never returns!)
	stackmaster();
    }
    // if we get here, stackmaster did a longjmp back to us.
}


WvTaskMan::~WvTaskMan()
{    
    magic_number = -42;
}


WvTask *WvTaskMan::start(WvStringParm name, 
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
    assert(magic_number == -WVTASK_MAGIC);
    assert(task.magic_number == WVTASK_MAGIC);
    assert(!task.recycled);
    
    if (&task == current_task)
	return val; // that's easy!
        
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
	assert(magic_number == -WVTASK_MAGIC);
	assert(task.magic_number == WVTASK_MAGIC);
	
	// initial setup
	stack_target = &task;
	longjmp(stackmaster_task, size/1024 + (size%1024 > 0));
    }
    else
    {
	assert(magic_number == -WVTASK_MAGIC);
	assert(task.magic_number == WVTASK_MAGIC);
	
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
    
    for (;;)
    {
	assert(magic_number == -WVTASK_MAGIC);
	
	val = setjmp(stackmaster_task);
	if (val == 0)
	{
	    assert(magic_number == -WVTASK_MAGIC);
	    
	    // just did setjmp; save stackmaster's current state (with
	    // all current stack allocations) and go back to get_stack
	    // (or the constructor, if that's what called us)
	    longjmp(get_stack_return, 1);
	}
	else
	{
	    assert(magic_number == -WVTASK_MAGIC);
	    
	    // set up a stack frame for the task
	    do_task();
	    
	    assert(magic_number == -WVTASK_MAGIC);
	    
	    // allocate the stack area so we never use it again
	    alloca(val * (size_t)1024);
	}
    }
}


void WvTaskMan::do_task()
{
    assert(magic_number == -WVTASK_MAGIC);
    WvTask *task = stack_target;
    assert(task->magic_number == WVTASK_MAGIC);
	
    // back here from longjmp; someone wants stack space.    
    if (setjmp(task->mystate) == 0)
    {
	// done the setjmp; that means the target task now has
	// a working jmp_buf all set up.  Leave space on the stack
	// for his data, then repeat the loop (so we can return
	// to get_stack(), and allocate more stack for someone later)	
	return;
    }
    else
    {
	// someone did a run() on the task, which
	// means they're ready to make it go.  Do it.
	for (;;)
	{	    
	    assert(magic_number == -WVTASK_MAGIC);
	    assert(task->magic_number == WVTASK_MAGIC);
	    
	    if (task->func && task->running)
	    {
		task->func(task->userdata);
		task->name = "DEAD";
		task->running = false;
		task->numrunning--;
	    }
	    yield();
	}
    }
}
