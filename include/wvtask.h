/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A set of classes that provide co-operative multitasking support.  By
 * default there's no scheduler -- you have to provide it yourself.  As it
 * stands, this is just a convenient way to switch from one context to
 * another when you know exactly what you want to do.
 * 
 * This is mainly intended for use by WvStream, but that's probably not the
 * only possible use... see also WvCont.
 */
#ifndef __WVTASK_H
#define __WVTASK_H

#ifdef _WIN32

#include "wvwin32task.h"

#else

#include "wvstring.h"
#include "wvlinklist.h"
#include <ucontext.h>

#define WVTASK_MAGIC 0x123678

class WvTaskMan;

/** Represents a single thread of control. */
class WvTask
{
    friend class WvTaskMan;
    
    // you might think it would be useful to have this return an int, since
    // yield() and run() both return int.  But that ends up being more
    // confusing than you think, because if you call task1->run(), and he
    // calls task2->run(), and task2 calls yield(), then task1->run() returns
    // the value *task2* passed to yield!  So we avoid the confusion by not
    // using return values here, which discourages people from thinking of
    // them as return values.
    typedef void TaskFunc(void *userdata);
    
    static int taskcount, numtasks, numrunning;
    int magic_number, *stack_magic;
    WvString name;
    int tid;
    
    size_t stacksize;
    void *stack;
    bool running, recycled;
    
    WvTaskMan &man;
    ucontext_t mystate;	// used for resuming the task
    ucontext_t func_call, func_return;
    
    TaskFunc *func;
    void *userdata;
    
    WvTask(WvTaskMan &_man, size_t _stacksize = 64*1024);
    
public:
    virtual ~WvTask();
    
    void start(WvStringParm _name, TaskFunc *_func, void *_userdata);
    bool isrunning() const
        { return running; }
    void recycle();
};


DeclareWvList(WvTask);

/** Provides co-operative multitasking support among WvTask instances. */
class WvTaskMan
{
    friend class WvTask;
    
    static WvTaskMan *singleton;
    static int links;
    
    static int magic_number;
    static WvTaskList free_tasks;
    
    static void get_stack(WvTask &task, size_t size);
    static void stackmaster();
    static void _stackmaster();
    static void do_task();
    static void call_func(WvTask *task);

    static char *stacktop;
    static ucontext_t stackmaster_task;
    
    static WvTask *stack_target;
    static ucontext_t get_stack_return;
    
    static WvTask *current_task;
    static ucontext_t toplevel;
    
    WvTaskMan();
    virtual ~WvTaskMan();

#ifdef ENABLE_DELETE_DETECTOR
    friend void operator &&<WvTaskMan>(CheckIObject, const WvTaskMan *);
#endif
    
public:
    /// get/dereference the singleton global WvTaskMan
    static WvTaskMan *get();
    static void unlink();
    
    WvTask *start(WvStringParm name,
		  WvTask::TaskFunc *func, void *userdata,
		  size_t stacksize = 64*1024);
    
    // run() and yield() return the 'val' passed to run() when this task
    // was started.
    static int run(WvTask &task, int val = 1);
    static int yield(int val = 1);
    
    static WvTask *whoami()
        { return current_task; }
};


#endif // ifdef _WIN32
#endif // __WVTASK_H
