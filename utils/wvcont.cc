/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvCont provides "continuations", which are apparently also known as
 * semi-coroutines.  See wvcont.h for more details.
 */
#include "wvcont.h"
#include "wvtask.h"
#include "wvlinklist.h"
#include <assert.h>
 
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
    
    WvContCallback cb;        // the callback we want to call inside our WvTask
    void *ret;
    void *p1;
    
    Data(const WvContCallback &_cb, size_t _stacksize) : cb(_cb)
        { links = 1; finishing = false; stacksize = _stacksize; mydepth = 0;
	     taskman = WvTaskMan::get(); 
	     task = NULL; report();
        if (data_list == NULL)
            data_list = new DataList;
        data_list->append(this, false);
        }
    ~Data();

    void link()
        { links++; report(); }
    void unlink()
        { links--; report(); if (!links) delete this; }
    
    void report()
        { /* printf("%p: links=%d\n", this, links); */ }
};


WvCont::Data *WvCont::curdata = NULL;
int WvCont::taskdepth = 0;


WvCont::DataList *WvCont::data_list = NULL;


WvCont::WvCont(const WvCont &cb)
{
    static bool first = true;
    if (first)
    {
        first = false;
        WvStreamsDebugger::add_command("conts", 0,
                debugger_conts_run_cb, 0);
    }

    data = cb.data;
    data->link();
}


WvCont::WvCont(const WvContCallback &cb, unsigned long _stacksize)
{
    data = new Data(cb, (size_t)_stacksize);
}


WvCont::WvCont(Data *data)
{
    this->data = data;
    data->link();
}


WvCont::~WvCont()
{
    if (data->links == 1) // I'm the last link, and it's not currently running
    {
	data->finishing = true;
	data->p1 = NULL; // don't re-pass invalid data
	while (data->task && data->task->isrunning())
	    call();
    }
    
    data->unlink();
}


WvCont::Data::~Data()
{
    assert(!links);
    
    if (task)
	task->recycle();
    taskman->unlink();
    //printf("%p: deleting\n", this);
    report();

    data_list->unlink(this);
    if (data_list->isempty())
    {
        delete data_list;
        data_list = NULL;
    }
}


static inline const char *Yes_No(bool val)
{
    return val? "Yes": "No";
}


WvString WvCont::debugger_conts_run_cb(WvStringParm cmd, WvStringList &args,
        WvStreamsDebugger::ResultCallback result_cb, void *)
{
    const char *format = "%5s%s%5s%s%9s%s%10s%s%7s%s%s";
    WvStringList result;
    result.append(format, "Links", "-", "Depth", "-", "Finishing", "-", "Stack Size",
            "-", "Task ID", "-", "Task Name------");
    result_cb(cmd, result);
    
    if (!data_list)
        return WvString::null;

    DataList::Iter i(*data_list);
    for (i.rewind(); i.next(); )
    {
        result.zap();
        result.append(format,
                i->links, " ", i->mydepth, " ", Yes_No(i->finishing), " ",
                i->stacksize, " ",
                i->task? WvString(i->task->get_tid()): WvString("n/a"), " ",
                i->task? i->task->get_name(): WvString("n/a"));
        result_cb(cmd, result);
    }

    return WvString::null;
}


// note: assumes data->task is already running!
void *WvCont::_call(Data *data)
{
    Data *olddata = curdata;
    curdata = data;
    data->link(); // don't delete this context while it's running!
    
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
	assert(data->task);
	do
	{
	    data->taskman->run(*data->task);
	    if (data->links == 1)
	    {
		data->finishing = true; // make WvCont::isok() false
		data->p1 = NULL; // don't re-pass invalid data
	    }
	} while (data->finishing && data->task && data->task->isrunning());
	assert(data->links);
    } while (taskdepth > data->mydepth);
    assert(taskdepth == data->mydepth);
    taskdepth--;
    data->mydepth = 0;

    void *ret = data->ret;
    data->unlink();
    curdata = olddata;
    return ret;
}


void *WvCont::operator() (void *p1)
{
    data->ret = reinterpret_cast<void*>(-42);
    
    if (!data->task)
	data->task = data->taskman->start("wvcont", bouncer, data,
					  data->stacksize);
    else if (!data->task->isrunning())
	data->task->start("wvcont+", bouncer, data);

    assert(data->task);
    
    data->p1 = p1;
    return call();
}


WvCont WvCont::current()
{
    assert(curdata);
    assert(curdata->task == curdata->taskman->whoami());
    assert(isok()); // this assertion is a bit aggressive...
    return WvCont(curdata);
}


void *WvCont::yield(void *ret)
{
    assert(curdata);
    assert(curdata->task == curdata->taskman->whoami());
    
    // this assertion is a bit aggressive, but on purpose; a callback that
    // does yield() instead of returning when its context should be dying
    // is pretty badly behaved.
    assert(isok());
    
    curdata->ret = ret;
    curdata->taskman->yield();
    return curdata->p1;
}


bool WvCont::isok()
{
    // if we're not using WvCont, it's not okay to yield
    if (!curdata)
	return false;

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
