/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvCont provides "continuations", which are apparently also known as
 * semi-coroutines.  See wvcont.h for more details.
 */
#include "wvcont.h"
#include "wvtask.h"
 
// private data that doesn't need to be in the header
struct WvCont::Data
{
    int links;          // the refcount of this Data object
    int mydepth;        // this task's depth in the call stack
    bool finishing;     // true if we're trying to terminate this task ASAP
                        //     (generally because WvCont is being destroyed)
    size_t stacksize;
    WvTaskMan *taskman;
    WvTask *task;
    
    Callback cb;        // the callback we want to call inside our WvTask
    R ret;
    P1 p1;
    
    Data(const Callback &_cb, size_t _stacksize) : cb(_cb)
        { links = 1; finishing = false; stacksize = _stacksize; mydepth = 0;
	     taskman = WvTaskMan::get(); 
	     task = NULL; report(); }
    ~Data()
        { assert(!links); taskman->unlink(); report(); }

    void link()
        { links++; report(); }
    void unlink()
        { links--; report(); if (!links) delete this; }
    
    void report()
        { /* printf("%p: links=%d\n", this, links); */ }
};


WvCont::Data *WvCont::curdata = NULL;
int WvCont::taskdepth = 0;


WvCont::WvCont(const WvCont &cb)
{
    data = cb.data;
    data->link();
}


WvCont::WvCont(const Callback &cb, unsigned long _stacksize)
{
    data = new Data(cb, (size_t)_stacksize);
}


WvCont::~WvCont()
{
    if (data->links == 1)
    {
	// run the task until it finishes.  We can't delete it until then!
	data->finishing = true; // make WvCont::isok() false
	while (data->task && data->task->isrunning())
	    call();
	
	if (data->task)
	    data->task->recycle();
    }
    data->unlink();
}


// note: assumes data->task is already running!
void WvCont::call()
{
    Data *olddata = curdata;
    curdata = data;
    
    // enforce the call stack.  If we didn't do this, a yield() five calls
    // deep would return to the very top, rather to the second-innermost
    // context.
    // 
    // Note that this implementation has the interesting side-effect of
    // short-circuiting recursion (a calls b, b calls c, c calls a), since
    // calling 'a' if it's already running means the same as "yield all the
    // way back to a", and this loop enforces one-level-at-a-time yielding.
    // 
    // Because that behaviour is probably undesirable, we make 'mydepth' into
    // a member variable instead of just putting it on the stack.  This is
    // only needed so that we can have the assert().
    assert(!data->mydepth);
    data->mydepth = ++taskdepth;
    do
    {
	data->taskman->run(*data->task);
    } while (taskdepth > data->mydepth);
    assert(taskdepth == data->mydepth);
    taskdepth--;
    data->mydepth = 0;

    curdata = olddata;
}


WvCont::WvCont(Data *data)
{
    this->data = data;
    data->link();
}


WvCont::R WvCont::operator() (P1 p1)
{
    data->ret = R(-42);
    
    if (!data->task)
	data->task = data->taskman->start("wvcont", bouncer, data,
					  data->stacksize);
    else if (!data->task->isrunning())
	data->task->start("wvcont+", bouncer, data);

    data->p1 = p1;
    call();
    return data->ret;
}


WvCont WvCont::current()
{
    assert(curdata);
    assert(curdata->task == curdata->taskman->whoami());
    assert(isok()); // this assertion is a bit aggressive...
    return WvCont(curdata);
}


WvCont::P1 WvCont::yield(R ret)
{
    assert(curdata);
    assert(curdata->task == curdata->taskman->whoami());
    assert(isok()); // this assertion is a bit aggressive...
    curdata->ret = ret;
    curdata->taskman->yield();
    return curdata->p1;
}


bool WvCont::isok()
{
    assert(curdata);
    assert(curdata->task == curdata->taskman->whoami());
    return !curdata->finishing;
}


void WvCont::bouncer(void *userdata)
{
    Data *data = (Data *)userdata;
    
    // DON'T BE FOOLED!
    // all yield() calls stay inside the inner function; our return value
    // is only for the final run after data->cb() returns.
    data->ret = data->cb(data->p1);
}

    
