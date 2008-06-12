#include "wvtr1.h"
#include <stdio.h>

struct A
{
    long x, y;
    A(long _x = 0, long _y = 0)
        { x = _x; y = _y; }
    
    A add(const A &a)
        { return A(x+a.x, y+a.y); }
};

typedef wv::function<A(const A&, void*)> ACallback;
typedef wv::function<A(const A&)> A2Callback;
typedef wv::function<A(void *)> A3Callback;


static A bunk(const A &a, void *userdata)
{
    long incr = (long)userdata;
    return A(a.x+incr, a.y+incr*2);
}


// one-parameter version of bunk()
static A bunk1(const A &a)
{
    return bunk(a, (void *)1);
}


static void print_a(const A &a)
{
    printf("result: %d/%d\n", a.x, a.y);
}


int main()
{
    A a(1000, 2000);
    
    ACallback c0(bunk);
    A2Callback c1(bunk1);
    // FIXME: I am broken. Please show this to somebody
    // who can fix.... can fix.... can fix.....
    // A3Callback c2(WvBoundCallback<A3Callback, const A &>(bunk, a));
    A2Callback c3(wv::bind(&A::add, &a, _1));
    
    print_a(c0(a, (void *)5));
    print_a(c1(a));
    // print_a(c2((void *)2));
    print_a(c3(a));
    
    return 0;
}
