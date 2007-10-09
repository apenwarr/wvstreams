#include "wvtest.h"
#include "wvtr1.h"
#include <stdio.h>

// START callbacktest.cc definitions
struct A
{
    int x, y;
    A(int _x = 0, int _y = 0)
        { x = _x; y = _y; }
   
    A add(const A &a)
        { return A(x+a.x, y+a.y); }
};

typedef wv::function<A(const A&, void*)> ACallback;
typedef wv::function<A(const A&)> A2Callback;
typedef wv::function<A(void*)> A3Callback;


static A bunk(const A &a, void *userdata)
{
    int incr = (int)userdata;
    return A(a.x+incr, a.y+incr*2);
}


// one-parameter version of bunk()
static A bunk1(const A &a)
{
    return bunk(a, (void *)1);
}

typedef wv::function<void()> Cb;
typedef wv::function<int()> ICb;

void f()
{
    // nothing
}


class Derived : public Cb
{
public:
    Derived(const Cb &cb) : Cb(cb)
        { }
};


static int ginstance;

class Functor
{
public:
    int instance;
    
    Functor() 
        { instance = ++ginstance; }
    Functor(const Functor &f) 
        { instance = ++ginstance; }
    
    int operator()() 
        { return instance; }
};



WVTEST_MAIN("callbacktest.cc")
{
    {
        A a(1000, 2000);
        A result;
    
        ACallback c0(bunk);
        A2Callback c1(bunk1);
        A3Callback c2(wv::bind(&bunk, a, _1));
        A2Callback c3(wv::bind(&A::add, &a, _1));
    
        result = (c0(a, (void *)5));
        if (!WVPASS(result.y == 2 * result.x))
            printf("   because [%d] != 2 * [%d]\n", result.y, result.x);
        
        result = (c1(a));
        if (!WVPASS(result.y == 2 * result.x))
            printf("   because [%d] != 2 * [%d]\n", result.y, result.x);
        
        result = (c2((void *)2));
        if (!WVPASS(result.y == 2 * result.x))
            printf("   because [%d] != 2 * [%d]\n", result.y, result.x);        
        
        result = (c3(a));
        if (!WVPASS(result.y == 2 * result.x))
            printf("   because [%d] != 2 * [%d]\n", result.y, result.x);
    }
}


WVTEST_MAIN("cbweirdtest")
{
    {
        Cb cb1(f);
        Cb cb2(cb1);
        Derived cb3(f);
        Derived cb4(cb1);
        Derived cb5(cb3);
	cb5();
	cb4();
    }
    WVPASS(true);

    // test that instantiating WvCallback from a functor object actually
    // copies that object, it doesn't just take a reference.
    // This is relevant for WvCont.
    {
	Functor ff;
	WVPASSEQ(ff(), 1);
	WVPASSEQ(ff(), 1);
	ICb *cb1 = new ICb(ff);
	ICb *cb2 = new ICb(*cb1);
	int c1 = (*cb1)();
	WVPASS(c1 > 1);
	delete cb1;
	WVPASS((*cb2)() > c1);
	delete cb2;
    }
    WVPASS(true);
}
