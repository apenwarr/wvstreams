#include "wvtest.h"
#include "wvcont.h"
#include <stdio.h>

// START conttest.cc definitions
static void *nonfunc(void *_x)
{
    int x = (int)_x;
    
    return (void *)(1234560000 + x);
}


static void *func(void *_x)
{
    int x = (int)_x;
    
    for (int count = 0; count < 4 && WvCont::isok(); count++)
	WvCont::yield((void *)++x);
    return (void *) -(++x);
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
    void *honker(Honk &h, void *_x)
    {
	int x = (int)_x;
//	printf("%s: STARTING (%d)\n", id, x);
	
	for (x--; WvCont::isok() && x > 0; x--)
	{
//	    printf("%s: --> Honking in (%d)\n", id, x);
	    h.cb((void *)x);
//	    printf("%s: <-- Honking out (%d)\n", id, x);
	}
	
//	printf("%s: DONE\n", id);
	return (void *)x;
    }
};
// END conttest.cc definitions

WVTEST_MAIN("basics")
{
    typedef WvCallback<void *, void *> CbType;
    
    // basic functionality (including nested tasks)
    {
	CbType cbx = func; // not runnable itself: no yield allowed
	CbType cb1 = WvCont(cbx); // a subtask
	CbType cb2 = WvCont(cb1); // another subtask on top
	CbType cb3 = cb2; // a copy of the second subtask
	// note that in the above, there's really only one context in which
	// 'func' actually gets called; there are no parallel running 'func's.
	// cb2's task calls into cb1's task, however.
    
	WVPASS((int)cb1((void *)100) == 101);
	WVPASS((int)cb2((void *)200) == 102);
	WVPASS((int)cb3((void *)300) == 103);
	cb1 = WvCont(nonfunc);
	WVPASS((int)cb1((void *)400) == 1234560400);
	WVPASS((int)cb2((void *)500) == 104);
	WVPASS((int)cb3((void *)600) == -105);
	cb2 = nonfunc;
        WVPASS((int)cb1((void *)700) == 1234560700);
	WVPASS((int)cb2((void *)800) == 1234560800);
	WVPASS((int)cb3((void *)900) == 901);
	cb3 = nonfunc;
	WVPASS((int)cb1((void *)1000) == 1234561000);
	WVPASS((int)cb2((void *)1100) == 1234561100);
	WVPASS((int)cb3((void *)1200) == 1234561200);
    }

#ifdef CAN_UNITTEST_ASSERTION_FAILURE 
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
	
	h1.cb((void *)5);
    }
    
    return 0;
#else
#warning "Assertion failure test not converted"
#endif
}


static WvCallback<void*,void*> rcallback;
static int rn = 0;

static void *rfunc2(void *)
{
    rn++;
    WVPASS(rn == 3);
    return 0;
}

static void *rfunc1(void *)
{
    rn++;
    WVPASS(rn == 1);
    rcallback = WvCont(rfunc2);
    WVPASS(WvCont::isok()); // not dead until after we yield once!
    WVPASS(rn == 1);
    WvCont::yield();
    WVPASS(!WvCont::isok());
    rn++;
    WVPASS(rn == 2);
    return 0;
}


WVTEST_MAIN("self-redirection")
{
    rcallback = WvCont(rfunc1);
    rcallback(0);
    rcallback(0);
}
