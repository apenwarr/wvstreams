#include "wvtask.h"
#include "wvcallback.h"
#include <stdio.h>


class WvCont
{
public:
    /**
     * These are hardcoded to int, because I'm too lazy to templatize this.
     * Most people won't use the return and parameter values anyhow.
     */
    typedef int R;
    typedef int P1;

    typedef WvCallback<R, P1> Callback;
    
private:
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
    WvCont(const Callback &cb);
    
    /** Copy constructor. */
    WvCont(const WvCont &cb);
    
    /** Destructor. */
    ~WvCont();
    
    /**
     * call the callback, making p1 the return value of yield() or the
     * parameter to the function, and returning Ret, the argument of yield()
     * or the return value of the function.
     */
    R operator() (P1 p1);
    
    // the following are static because a function doesn't really know
    // which WvCont it belongs to, and only one WvCont can be the "current"
    // one globally in an application anyway.
    // 
    // Unfortunately this prevents us from assert()ing that you're in the
    // context you think you are.
    
    /**
     * "return" from the current callback, giving value 'ret' to the person
     * who called us.  Next time this callback is called, it's as if yield()
     * had returned, and the parameter to the callback is the value of
     * yield().
     */
    static P1 yield(R ret);
    
    /**
     * Tell us if the current context is "okay", that is, not trying to
     * die. If !isok(), you shouldn't yield(), because the caller is just
     * going to keep calling you until you die.  Return as soon as you can.
     */
    static bool isok();
};


struct WvCont::Data
{
    int links;          // the refcount of this Data object
    int mydepth;        // this task's depth in the call stack
    bool finishing;     // true if we're trying to terminate this task ASAP
    WvTaskMan *taskman;
    WvTask *task;
    
    Callback cb;        // the callback we want to call inside our WvTask
    R ret;
    P1 p1;
    
    Data(const Callback &_cb) : cb(_cb)
        { links = 1; taskman = WvTaskMan::get(); 
	     task = NULL; finishing = false; report(); }
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


WvCont::WvCont(const Callback &cb)
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


WvCont::R WvCont::operator() (P1 p1)
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

    
static int nonfunc(int x)
{
    return 1234560000 + x;
}


static int func(int x)
{
    for (int count = 0; count < 4 && WvCont::isok(); count++)
	WvCont::yield(++x);
    return -(++x);
}


class Honk
{
public:
    const char *id;
    
    WvCont::Callback cb;
    
    Honk(const char *_id)
        { id = _id; }
    
    void honk_at(Honk &a)
    {
	cb = WvCont(WvBoundCallback<WvCont::Callback, Honk &>
		    (this, &Honk::honker, a));
    }

private:
    int honker(Honk &h, int x)
    {
	printf("%s: STARTING (%d)\n", id, x);
	
	for (x--; WvCont::isok() && x > 0; x--)
	{
	    printf("%s: --> Honking in (%d)\n", id, x);
	    h.cb(x);
	    printf("%s: <-- Honking out (%d)\n", id, x);
	}
	
	printf("%s: DONE\n", id);
	return x;
    }
};


int main()
{
    typedef WvCallback<int, int> CbType;
    
    // basic functionality (including nested tasks)
    {
	CbType cbx = func; // not runnable itself: no yield allowed
	CbType cb1 = WvCont(cbx); // a subtask
	CbType cb2 = WvCont(cb1); // another subtask on top
	CbType cb3 = cb2; // a copy of the second subtask
	// note that in the above, there's really only one context in which
	// 'func' actually gets called; there are no parallel running 'func's.
	// cb2's task calls into cb1's task, however.
    
	printf("zot1: %d\n", cb1(100));
	printf("zot1: %d\n", cb2(200));
	printf("zot1: %d\n", cb3(300));
	cb1 = WvCont(nonfunc);
	printf("zot2: %d\n", cb1(400));
	printf("zot2: %d\n", cb2(500));
	printf("zot2: %d\n", cb3(600));
	cb2 = nonfunc;
	printf("zot3: %d\n", cb1(700));
	printf("zot3: %d\n", cb2(800));
	printf("zot3: %d\n", cb3(900));
	cb3 = nonfunc;
	printf("zot4: %d\n", cb1(1000));
	printf("zot4: %d\n", cb2(1100));
	printf("zot4: %d\n", cb3(1200));
    }
    
    // fun with recursive continuations.  If this doesn't do something
    // predictable, we'll get screwy bugs when we use this in WvStreams - just
    // like we did with the pre-WvCont continue_select() implementation.
    // 
    // The *desired* behaviour here is the same as with real recursive
    // function calls: if a calls b who calls c, and then c calls a again,
    // then a should do its thing, return (or yield), then c will finish,
    // yield, then b will finish, yield, and then a will have a chance to run
    // again.
    // 
    // In old wvstreams, we would silently short-circuit the recursion (the
    // inner a would yield immediately without doing anything).  This is
    // easy to implement, but causes problems if c actually expects a to do
    // something.
    // 
    // Unfortunately, the semantics of this are tricky with continuations:
    // when we call the inner a, we re-enter its context, but that context
    // is waiting for b to return.  It can't do anything unless b returns,
    // so what can we do?
    // 
    // ...we assert() instead.  So expect an assertion failure below.
    printf("Expect an assertion failure shortly!\n");
    {
	Honk h1("honk1"), h2("honk2"), h3("honk3");
	h1.honk_at(h2);
	h2.honk_at(h3);
	h3.honk_at(h1);
	
	h1.cb(5);
    }
    
    return 0;
}
