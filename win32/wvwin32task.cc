/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 */
#define WIN32_LEAN_AND_MEAN
//#define NOMINMAX
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <windows.h>
#include <winbase.h>

#include "wvwin32task.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h> // for alloca()
#include <stdlib.h> // for alloca() on non-Linux platforms?

int WvTask::taskcount, WvTask::numtasks, WvTask::numrunning;
struct WvTaskManSingleton {
    DWORD threadid;
    WvTaskMan *taskman;
    int links; // never delete singleton
};
static WvTaskManSingleton singletons[10];

int WvTaskMan::magic_number;
WvTaskList WvTaskMan::free_tasks;
    
WvTask *WvTaskMan::stack_target;
    
WvTask *WvTaskMan::current_task;
LPVOID WvTaskMan::toplevel;

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

    mystate = CreateFiber(_stacksize, &MyFiberProc, this);
    assert(mystate);
}


WvTask::~WvTask()
{
    numtasks--;
    if (running)
	numrunning--;
    magic_number = 42;
}

VOID CALLBACK WvTask::MyFiberProc(PVOID lpParameter)
{
    WvTask *_this = (WvTask *) lpParameter;
    while (true)
    {
	if (_this->func && _this->running)
	{
	    _this->func(_this->userdata);

	    // the task's function terminated.
	    _this->name = "DEAD";
	    _this->running = false;
	    _this->numrunning--;
	}
	_this->man.yield();
    }
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

// FIXME: this is mildly racy, although only if the *first* call to get() on
// a particular thread is at the same time as on another thread.  Since any
// sane WvStreams program will only have one thread using WvTaskMan anyway,
// except in a few obscure test situations, I don't consider this a big problem.
WvTaskMan *WvTaskMan::get()
{
    DWORD tid = GetCurrentThreadId();
    fprintf(stderr, "WvTaskMan::get: tid=%ld\n", tid);
    
    for (unsigned i = 0; i < sizeof(singletons) / sizeof(singletons[0]); i++)
    {
	if (singletons[i].threadid == tid)
	{
	    singletons[i].taskman->links++;
	    return singletons[i].taskman;
	}
    }
    
    // none found?
    fprintf(stderr, "NEW SINGLETON TASKMAN\n");
    for (unsigned i = 0; i < sizeof(singletons) / sizeof(singletons[0]); i++)
    {
	if (!singletons[i].taskman) // empty slot
	{
	    singletons[i].taskman = new WvTaskMan;
	    singletons[i].threadid = tid;
	    return singletons[i].taskman;
	}
    }

    assert(0); // no slots available
    return NULL;
}


void WvTaskMan::unlink()
{
    links--;
    if (links == 0)
    {
	for (unsigned i = 0; i < sizeof(singletons) / sizeof(singletons[0]); i++)
	{
	    if (singletons[i].taskman == this)
	    {
		fprintf(stderr, "FREEING TASKMAN (links=0)\n");
		delete this;
		singletons[i].taskman = NULL;
		singletons[i].threadid = 0;
	    }
	}
    }
}

WvTaskMan::WvTaskMan()
{
    links = 1;
    stack_target = NULL;
    current_task = NULL;
    magic_number = -WVTASK_MAGIC;
    
    toplevel = ::ConvertThreadToFiber(0);
    // On XP 64, we're already a fiber... somehow (??).  Instead of
    // erroring out, though, just get the current fiber.
    // Error codes here:
    // http://msdn.microsoft.com/en-us/library/ms681383(VS.85).aspx
    if (!toplevel && GetLastError() == ERROR_ALREADY_FIBER)
	toplevel = GetCurrentFiber();
    assert(toplevel);
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
	    i.link->set_autofree(false);
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
    LPVOID *state;
    
    if (!old_task)
	state = &toplevel; // top-level call (not in an actual task yet)
    else
	state = &old_task->mystate;
    
    //fprintf(stderr, "taskman: switching from %p to %p\n", old_task, &task);
    ::SwitchToFiber(task.mystate);
    //fprintf(stderr, "taskman: back in %p\n", old_task);
    
    // someone did yield() (if toplevel) or run() on our task; exit
    current_task = old_task;
    int newval = 0;
    return newval;
}


int WvTaskMan::yield(int val)
{
    if (!current_task)
	return 0; // weird...
    
    WvTask *task = current_task;
    //fprintf(stderr, "taskman: yielding from %p to toplevel\n", task);
    current_task = 0; // toplevel
    ::SwitchToFiber(toplevel);
    //fprintf(stderr, "taskman: return from yield in %p (%p)\n", current_task, task);
    assert(current_task == task);

    int newval = 0;
    return newval;
}
