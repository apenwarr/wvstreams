#pragma once
#include "wvstring.h"
#include "wvlinklist.h"

#include <windows.h>

#define WVTASK_MAGIC 0x123678

class WvTaskMan;

class WvTask
{
    friend class WvTaskMan;
    typedef void TaskFunc(void *userdata);
    
    static int taskcount, numtasks, numrunning;
    int magic_number, *stack_magic;
    WvString name;
    int tid;
    
    size_t stacksize;
    bool running, recycled;
    
    WvTaskMan &man;
    LPVOID mystate;	// used for resuming the task
    
    TaskFunc *func;
    void *userdata;

    static VOID CALLBACK MyFiberProc(PVOID lpParameter);
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
    int magic_number;
    WvTask *current_task;
    WvTaskList free_tasks;
    
    void get_stack(WvTask &task, size_t size);
    void stackmaster();
    void _stackmaster();
    void do_task();
    
    WvTask *stack_target;
    
    LPVOID toplevel;
    
public:
    WvTaskMan();
    virtual ~WvTaskMan();
    
    WvTask *start(WvStringParm name,
		  WvTask::TaskFunc *func, void *userdata,
		  size_t stacksize = 64*1024);
    
    // run() and yield() return the 'val' passed to run() when this task
    // was started.
    int run(WvTask &task, int val = 1);
    int yield(int val = 1);
    
    WvTask *whoami() const
        { return current_task; }
};

