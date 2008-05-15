/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A set of classes that provide co-operative multitasking support.  See
 * wvtask.h for more information.
 */

#include "wvautoconf.h"
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  if HAVE_ALLOCA_H
#   include <alloca.h>
#  else
#   ifdef _AIX
#pragma alloca
#   else
#    ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#    endif
#   endif
#  endif
# endif
#endif

#include "wvtask.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>

#ifdef HAVE_VALGRIND_MEMCHECK_H
#include <valgrind/memcheck.h>
// Compatibility for Valgrind 3.1 and previous
#ifndef VALGRIND_MAKE_MEM_DEFINED
#define VALGRIND_MAKE_MEM_DEFINED VALGRIND_MAKE_READABLE 
#endif
#else
#define VALGRIND_MAKE_MEM_DEFINED(x, y)
#define RUNNING_ON_VALGRIND 0
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
WvTaskList WvTaskMan::all_tasks, WvTaskMan::free_tasks;
ucontext_t WvTaskMan::stackmaster_task, WvTaskMan::get_stack_return,
    WvTaskMan::toplevel;
WvTask *WvTaskMan::current_task, *WvTaskMan::stack_target;
char *WvTaskMan::stacktop;

static int context_return;


static bool use_shared_stack()
{
    return RUNNING_ON_VALGRIND;
}


static void valgrind_fix(char *stacktop)
{
#ifdef HAVE_VALGRIND_MEMCHECK_H
    char val;
    //printf("valgrind fix: %p-%p\n", &val, stacktop);
    assert(stacktop > &val);
#endif
    VALGRIND_MAKE_MEM_DEFINED(&val, stacktop - &val);
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

    man.all_tasks.append(this, false);
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


static inline const char *Yes_No(bool val)
{
    return val? "Yes": "No";
}


WvString WvTaskMan::debugger_tasks_run_cb(WvStringParm cmd, WvStringList &args,
        WvStreamsDebugger::ResultCallback result_cb, void *)
{
    const char *format_str = "%5s%s%7s%s%8s%s%6s%s%s";
    WvStringList result;
    result.append(format_str, "--TID", "-", "Running", "-", "Recycled", "-", "-StkSz", "-", "Name-----");
    result_cb(cmd, result);
    WvTaskList::Iter i(all_tasks);
    for (i.rewind(); i.next(); )
    {
        result.zap();
        result.append(format_str, i->tid, " ",
                Yes_No(i->running), " ",
                Yes_No(i->recycled), " ",
                i->stacksize, " ",
                i->name);
        result_cb(cmd, result);
    }
    return WvString::null;
}


WvTaskMan::WvTaskMan()
{
    static bool first = true;
    if (first)
    {
        first = false;
        WvStreamsDebugger::add_command("tasks", 0, debugger_tasks_run_cb, 0);
    }

    stack_target = NULL;
    current_task = NULL;
    magic_number = -WVTASK_MAGIC;
    
    stacktop = (char *)alloca(0);
    
    context_return = 0;
    assert(getcontext(&get_stack_return) == 0);
    if (context_return == 0)
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
	    i.set_autofree(false);
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
    ucontext_t *state;
    
    if (!old_task)
	state = &toplevel; // top-level call (not in an actual task yet)
    else
	state = &old_task->mystate;
    
    context_return = 0;
    assert(getcontext(state) == 0);
    int newval = context_return;
    if (newval == 0)
    {
	// saved the state, now run the task.
        context_return = val;
        setcontext(&task.mystate);
        return -1;
    }
    else
    {
        // need to make state readable to see if we need to make more readable..
        VALGRIND_MAKE_MEM_DEFINED(&state, sizeof(state));
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
    VALGRIND_MAKE_MEM_DEFINED(current_task->stack_magic,
                           sizeof(current_task->stack_magic));
    assert(*current_task->stack_magic == WVTASK_MAGIC);

#if TASK_DEBUG
    if (use_shared_stack())
    {
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
    }
#endif
		
    context_return = 0;
    assert(getcontext(&current_task->mystate) == 0);
    int newval = context_return;
    if (newval == 0)
    {
	// saved the task state; now yield to the toplevel.
        context_return = val;
        setcontext(&toplevel);
        return -1;
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
    context_return = 0;
    assert(getcontext(&get_stack_return) == 0);
    if (context_return == 0)
    {
	assert(magic_number == -WVTASK_MAGIC);
	assert(task.magic_number == WVTASK_MAGIC);

        if (!use_shared_stack())
        {
#if defined(__linux__) && (defined(__386__) || defined(__i386) || defined(__i386__))
            static char *next_stack_addr = (char *)0xB0000000;
            static const size_t stack_shift = 0x00100000;

            next_stack_addr -= stack_shift;
#else
            static char *next_stack_addr = NULL;
#endif
        
            task.stack = mmap(next_stack_addr, task.stacksize,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1, 0);
        }
	
	// initial setup
	stack_target = &task;
	context_return = size/1024 + (size%1024 > 0);
	setcontext(&stackmaster_task);
    }
    else
    {
	if (current_task)
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
	
        context_return = 0;
        assert(getcontext(&stackmaster_task) == 0);
        val = context_return;
	if (val == 0)
	{
	    assert(magic_number == -WVTASK_MAGIC);
	    
	    // just did setjmp; save stackmaster's current state (with
	    // all current stack allocations) and go back to get_stack
	    // (or the constructor, if that's what called us)
            context_return = 1;
            setcontext(&get_stack_return);
	}
	else
	{
	    valgrind_fix(stacktop);
	    assert(magic_number == -WVTASK_MAGIC);
	    
	    total = (val+1) * (size_t)1024;
	    
            if (!use_shared_stack())
                total = 1024; // enough to save the do_task stack frame

	    // set up a stack frame for the new task.  This runs once
	    // per get_stack.
            //alloc_stack_and_switch(total);
	    do_task();
	    
	    assert(magic_number == -WVTASK_MAGIC);

            // allocate the stack area so we never use it again
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


void WvTaskMan::call_func(WvTask *task)
{
    Dprintf("WvTaskMan: calling task #%d (%s)\n",
	    task->tid, (const char *)task->name);
    task->func(task->userdata);
    Dprintf("WvTaskMan: returning from task #%d (%s)\n",
	    task->tid, (const char *)task->name);
    context_return = 1;
}


void WvTaskMan::do_task()
{
    assert(magic_number == -WVTASK_MAGIC);
    WvTask *task = stack_target;
    assert(task->magic_number == WVTASK_MAGIC);
	
    // back here from longjmp; someone wants stack space.    
    context_return = 0;
    assert(getcontext(&task->mystate) == 0);
    if (context_return == 0)
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
                if (use_shared_stack())
                {
                    // this is the task's main function.  It can call yield()
                    // to give up its timeslice if it wants.  Either way, it
                    // only returns to *us* if the function actually finishes.
                    task->func(task->userdata);
                }
                else
                {
                    assert(getcontext(&task->func_call) == 0);
                    task->func_call.uc_stack.ss_size = task->stacksize;
                    task->func_call.uc_stack.ss_sp = task->stack;
                    task->func_call.uc_stack.ss_flags = 0;
                    task->func_call.uc_link = &task->func_return;
                    Dprintf("WvTaskMan: makecontext #%d (%s)\n",
                            task->tid, (const char *)task->name);
                    makecontext(&task->func_call,
                            (void (*)(void))call_func, 1, task);

                    context_return = 0;
                    assert(getcontext(&task->func_return) == 0);
                    if (context_return == 0)
                        setcontext(&task->func_call);
                }
		
		// the task's function terminated.
		task->name = "DEAD";
		task->running = false;
		task->numrunning--;
	    }
	    yield();
	}
    }
}


const void *WvTaskMan::current_top_of_stack()
{
    extern const void *__libc_stack_end;
    if (use_shared_stack() || current_task == NULL)
        return __libc_stack_end;
    else
        return (const char *)current_task->stack + current_task->stacksize;
}


size_t WvTaskMan::current_stacksize_limit()
{
    if (use_shared_stack() || current_task == NULL)
    {
        struct rlimit rl;
        if (getrlimit(RLIMIT_STACK, &rl) == 0)
            return size_t(rl.rlim_cur);
        else
            return 0;
    }
    else
        return size_t(current_task->stacksize);
}

    
