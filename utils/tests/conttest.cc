#include "wvtask.h"
#include "wvcallback.h"
#include <stdio.h>

typedef int Ret;
typedef int P1;
typedef WvCallback<Ret, P1> RealCallback;


class WvCont
{
    /**
     * When we copy a WvCont, we increase the reference count of the 'data'
     * member rather than copying it.  That makes it so every copy of a given
     * callback object still refers to the same WvTask.
     */
    struct Data;
    Data *data;

    static Data *curdata;
    static int taskdepth;
    
    static void bouncer(void *userdata);
    
    /**
     * Actually call the callback inside its task, and enforce a call stack.
     * Doesn't do anything with arguments or return values.
     */
    void call();
    
public:
    /**
     * Construct a WvCont using an existing WvCallback.  The WvCont object
     * can be used in place of that callback, and stored in a callback of
     * the same data type.
     */
    WvCont(const RealCallback &cb);
    
    /** Copy constructor. */
    WvCont(const WvCont &cb);
    
    /** Destructor. */
    ~WvCont();
    
    /**
     * call the callback, making p1 the return value of yield() or the
     * parameter to the function, and returning Ret, the argument of yield()
     * or the return value of the function.
     */
    Ret operator() (P1 p1);
    
    // the following are static because a function doesn't really know
    // which WvCont it belongs to, and only one WvCont can be the "current"
    // one globally in an application anyway.
    
    /**
     * "return" from the current callback, giving value 'ret' to the person
     * who called us.  Next time this callback is called, it's as if yield()
     * had returned, and the parameter to the callback is the value of
     * yield().
     */
    static P1 yield(Ret ret);
    static bool isok();
};


struct WvCont::Data
{
    int links;
    RealCallback cb;
    WvTaskMan *taskman;
    WvTask *task;
    bool finishing;
    Ret ret;
    P1 p1;
    
    Data(const RealCallback &_cb) : cb(_cb)
        { links = 1; taskman = WvTaskMan::get();
	     task = NULL; finishing = false; report(); }
    ~Data()
        { assert(!links); taskman->unlink(); report(); }
    void link()
        { links++; report(); }
    void unlink()
        { links--; report(); if (!links) delete this; }
    
    void report()
        { printf("%p: links=%d\n", this, links); }
};


WvCont::Data *WvCont::curdata = NULL;
int WvCont::taskdepth = 0;


WvCont::WvCont(const WvCont &cb)
{
    data = cb.data;
    data->link();
}


WvCont::WvCont(const RealCallback &cb)
{
    data = new Data(cb);
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


void WvCont::call()
{
    Data *olddata = curdata;
    curdata = data;
    
    // enforce the call stack.  If we didn't do this, a yield() five calls
    // deep would return to the very top.
    // 
    // There are more simple-minded ways to do this than using a stack, but
    // they can get you in trouble if you, for example, have a, which calls
    // b, which calls a, which calls c, which yields.
    // 
    // FIXME: something about this is insane.
    int mydepth = ++taskdepth;
    do
    {
	data->taskman->run(*data->task);
    } while (taskdepth > mydepth);
    assert(taskdepth == mydepth);
    taskdepth--;

    curdata = olddata;
}


int WvCont::operator() (P1 p1)
{
    data->ret = -42;
    
    if (!data->task)
	data->task = data->taskman->start("wvcont", bouncer, data);
    else if (!data->task->isrunning())
	data->task->start("wvcont+", bouncer, data);

    data->p1 = p1;
    call();
    return data->ret;
}


P1 WvCont::yield(Ret ret)
{
    assert(curdata);
    assert(curdata->task == curdata->taskman->whoami());
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
    
    // all yield() calls stay inside the inner function; our return value
    // is only for the final run after data->cb() returns.
    data->ret = data->cb(data->p1);
}

    
static int nonfunc(int x)
{
    return -1;
}


static int func(int x)
{
    for (int count = 0; count < 4 && WvCont::isok(); count++)
	WvCont::yield(++x);
    return -(++x);
}


int main()
{
    // basic functionality (including nested tasks)
    {
	RealCallback cbx = func; // not runnable itself: no yield allowed
	RealCallback cb1 = WvCont(cbx); // a subtask
	RealCallback cb2 = WvCont(cb1); // another subtask wrapping the first
	RealCallback cb3 = cb2; // a copy of the second subtask
	// note that in the above, there's really only one context in which
	// 'func' actually gets called; there are no parallel running 'func's.
    
	printf("zot1: %d\n", cb1(100));
	printf("zot1: %d\n", cb2(200));
	printf("zot1: %d\n", cb3(300));
	cb2 = nonfunc;
	printf("zot2: %d\n", cb1(400));
	printf("zot2: %d\n", cb2(500));
	printf("zot2: %d\n", cb3(600));
	cb1 = WvCont(nonfunc);
	printf("zot3: %d\n", cb1(700));
	printf("zot3: %d\n", cb2(800));
	printf("zot3: %d\n", cb3(900));
	cb3 = nonfunc;
	printf("zot4: %d\n", cb1(1000));
	printf("zot4: %d\n", cb2(1100));
	printf("zot4: %d\n", cb3(1200));
    }
    
    return 0;
}
