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
#include <stdlib.h> // for alloca() on non-Linux platforms?
#include "wvautoconf.h"
#ifdef HAVE_VALGRIND_MEMCHECK_H
#include <valgrind/memcheck.h>
#else
#define VALGRIND_MAKE_READABLE(x, y)
#endif

#define TASK_DEBUG 0
#if TASK_DEBUG
# define Dprintf(fmt, args...) fprintf(stderr, fmt, ##args)
#else
# define Dprintf(fmt, args...)
#endif

int WvTask::taskcount, WvTask::numtasks, WvTask::numrunning;

WvTaskMan *WvTaskMan::singleton;
int WvTaskMan::links, WvTaskMan::magic_number;
WvTaskList WvTaskMan::free_tasks;
jmp_buf WvTaskMan::stackmaster_task, WvTaskMan::get_stack_return,
    WvTaskMan::toplevel;
WvTask *WvTaskMan::current_task, *WvTaskMan::stack_target;
char *WvTaskMan::stacktop;


static void valgrind_fix(char *stacktop)
{
    char val;
    //printf("valgrind fix: %p-%p\n", &val, stacktop);
    assert(stacktop > &val);
    VALGRIND_MAKE_READABLE(&val, stacktop - &val);
}


WvTask::WvTask(WvTaskMan &_man, size_t _stacksize) : man(_man)
{
    stacksize = _stacksize;
    running = recycled = false;
    func = NULL;
    userdata = NULL;
    
    tid = ++taskcount;
    numtasks++;
    magic_number = WVTASK_MAGIC;
    stack_magic = NULL;
    
    man.get_stack(*this, stacksize);
}


WvTask::~WvTask()
{
    numtasks--;
    if (running)
	numrunning--;
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
    assert(!running);
    
    if (!running && !recycled)
    {
	man.free_tasks.append(this, true);
	recycled = true;
    }
}


WvTaskMan *WvTaskMan::get()
{
    if (!links)
	singleton = new WvTaskMan;
    links++;
    return singleton;
}


void WvTaskMan::unlink()
{
    links--;
    if (!links)
    {
	delete singleton;
	singleton = NULL;
    }
}


WvTaskMan::WvTaskMan()
{
    stack_target = NULL;
    current_task = NULL;
    magic_number = -WVTASK_MAGIC;
    
    stacktop = (char *)alloca(0);
    
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
    free_tasks.zap();
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
    
    Dprintf("WvTaskMan: running task #%d with value %d (%s)\n",
	    task.tid, val, (const char *)task.name);
    
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
        // need to make state readable to see if we need to make more readable..
        VALGRIND_MAKE_READABLE(&state, sizeof(state));
	// someone did yield() (if toplevel) or run() on our old task; done.
	if (state != &toplevel)
	    valgrind_fix(stacktop);
	current_task = old_task;
	return newval;
    }
}


int WvTaskMan::yield(int val)
{
    if (!current_task)
	return 0; // weird...
    
    Dprintf("WvTaskMan: yielding from task #%d with value %d (%s)\n",
	   current_task->tid, val, (const char *)current_task->name);
    
    assert(current_task->stack_magic);
    
    // if this fails, this task overflowed its stack.  Make it bigger!
    VALGRIND_MAKE_READABLE(current_task->stack_magic,
			   sizeof(current_task->stack_magic));
    assert(*current_task->stack_magic == WVTASK_MAGIC);

#if TASK_DEBUG
    size_t stackleft;
    char *stackbottom = (char *)(current_task->stack_magic + 1);
    for (stackleft = 0; stackleft < current_task->stacksize; stackleft++)
    {
	if (stackbottom[stackleft] != 0x42)
	    break;
    }
    Dprintf("WvTaskMan: remaining stack after #%d (%s): %ld/%ld\n",
	    current_task->tid, current_task->name.cstr(), (long)stackleft,
	    (long)current_task->stacksize);
#endif
		
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
	valgrind_fix(stacktop);
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
	valgrind_fix(stacktop);
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
    size_t total;
    
    Dprintf("stackmaster 1\n");
    
    // the main loop runs once from the constructor, and then once more
    // after each stack allocation.
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
	    valgrind_fix(stacktop);
	    assert(magic_number == -WVTASK_MAGIC);
	    
	    // set up a stack frame for the new task.  This runs once
	    // per get_stack.
	    do_task();
	    
	    assert(magic_number == -WVTASK_MAGIC);
	    
	    // allocate the stack area so we never use it again
	    total = (val+1) * (size_t)1024;
	    alloca(total);

	    // a little sentinel so we can detect stack overflows
	    stack_target->stack_magic = (int *)alloca(sizeof(int));
	    *stack_target->stack_magic = WVTASK_MAGIC;
	    
	    // clear the stack to 0x42 so we can count unused stack
	    // space later.
#if TASK_DEBUG
	    memset(stack_target->stack_magic + 1, 0x42, total - 1024);
#endif
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
	// for his data, then repeat the loop in _stackmaster (so we can
	// return to get_stack(), and allocate more stack for someone later)
	// 
	// Note that nothing on the allocated stack needs to be valid; when
	// they longjmp to task->mystate, they'll have a new stack pointer
	// and they'll already know what to do (in the 'else' clause, below)
	Dprintf("stackmaster 5\n");
	return;
    }
    else
    {
	// someone did a run() on the task, which
	// means they're ready to make it go.  Do it.
	valgrind_fix(stacktop);
	for (;;)
	{
	    assert(magic_number == -WVTASK_MAGIC);
	    assert(task);
	    assert(task->magic_number == WVTASK_MAGIC);
	    
	    if (task->func && task->running)
	    {
		// this is the task's main function.  It can call yield()
		// to give up its timeslice if it wants.  Either way, it
		// only returns to *us* if the function actually finishes.
		task->func(task->userdata);
		
		// the task's function terminated.
		task->name = "DEAD";
		task->running = false;
		task->numrunning--;
	    }
	    yield();
	}
    }
}
