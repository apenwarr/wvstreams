#include "wvcallback.h"
#include <stdio.h>

// test program for WvCallback
 
DeclareWvCallback(0, void, Cb0);
DeclareWvCallback(1, int, Cb1, int);

class EventProducer
{
public:

    Cb0 pushevent, pullevent;
    Cb1 chevent;
    
    EventProducer(Cb0 _pushevent, Cb0 _pullevent,
		  WvCallback1<int, int> _chevent)
	: pushevent(_pushevent), pullevent(_pullevent), chevent(_chevent)
	    { }
    void push() { printf("push: "); pushevent(); }
    void pull() { printf("pull: "); pullevent(); }
    void change(int x) { printf("change: "); printf("changed: %d\n", 
						    chevent(x)); }
};


class A
{
public:
    void f();
    int h(int x) { return 0;}
    int g(int x) { printf("in A::g (%d)\n", x); return x*2; }
};


void A::f()
{
    printf("in A::f\n"); g(-1);
}

class B : public A
{
public:
    virtual int h(int x) { return 0;}
    virtual int g(int x) { printf("in B::g (%d)\n", x); return x*3; }
};

typedef WvCallback0<void>::Fake Silly;

A global_a;
WvCallbackBase<void>::FakeFunc gfunc;

// experimental class like WvCallback0_bound, for playing with things to
// see what generates the smallest code.  reinterpret_cast makes a big
// difference...
class xWvCallback0_bound : public WvCallback0<void>
{
public:
    typedef void (A::*BoundFunc)();
    xWvCallback0_bound(A &_obj, BoundFunc _func)
	: WvCallback0<void>((Fake *)&_obj, reinterpret_cast<Func>(_func)) { }
};
 
// this isn't actually called.  We just look at its assembly output sometimes.
void very_simple()
{
    //WvCallback0<void> cb((Silly *)&global_a,
    //			 (WvCallback0<void>::Func)gfunc);
    
    //    xWvCallback0_bound cb(*(A *)&global_a,
    //			  (xWvCallback0_bound::BoundFunc)gfunc);

    //WvCallback0_bound<void,A> cb(*(A *)&global_a,
    //(WvCallback0_bound<void,A>::BoundFunc)gfunc);
    
    //VoidCallback_bound<A> cb(global_a, &A::f);
    //cb();
    
    VoidCallback cb(wvcallback(VoidCallback, global_a, A::f));
    cb();
}


int main()
{
    A a;
    B b;
    
    {
	printf("\ntest #1:\n");
	WvCallback0_bound<void, A> fp(a, &A::f);
	WvCallback0_bound<void, A> fp2(b, &A::f);
	WvCallback1_bound<int, B, int> gp(b, &B::g);
	
	fp();
	fp2();
	gp(gp(8));
    }
    
    {
	printf("\ntest #2:\n");
	EventProducer ev(wvcallback(Cb0, a, A::f),
			 wvcallback(Cb0, b, A::f),
			 wvcallback(Cb1, b, B::g));
	ev.push();
	ev.change(12);
	ev.chevent = wvcallback(Cb1, a, A::g);
	ev.pull();
	ev.change(5);
    }
    
    return 0;
}
