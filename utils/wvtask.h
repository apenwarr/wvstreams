/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * A set of classes that provide co-operative multitasking support.  By
 * default there's no scheduler -- you have to provide it yourself.  As it
 * stands, this is just a convenient way to switch from one context to
 * another when you know exactly what you want to do.
 * 
 * This is mainly intended for use by WvStream, but that's probably not the
 * only possible use...
 */
#ifndef __WVTASK_H
#define __WVTASK_H

#include "wvstring.h"
#include "wvlinklist.h"
#include "setjmp.h"

#define WVTASK_MAGIC 123678

class WvTaskMan;

class WvTask
{
    friend WvTaskMan;
    typedef void TaskFunc(void *userdata);
    
    static int taskcount, numtasks, numrunning;
    int magic_number;
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


DeclareWvList(WvTask);

class WvTaskMan
{
    friend WvTask;
    int magic_number;
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



#endif // __WVTASK_H
