#include "wvtest.h"
#include "wvcallback.h"
#include <stdio.h>

struct A
    {
        int x, y;
        A(int _x = 0, int _y = 0)
            { x = _x; y = _y; }
    
        A add(const A &a)
            { return A(x+a.x, y+a.y); }
    };

    typedef WvCallback<A, const A &, void *> ACallback;
    typedef WvCallback<A, const A &> A2Callback;
    typedef WvCallback<A, void *> A3Callback;


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

WVTEST_MAIN("old-style")
{
    {
        A a(1000, 2000);
        A result;
    
        ACallback c0(bunk);
        A2Callback c1(bunk1);
        A3Callback c2(WvBoundCallback<A3Callback, const A &>(bunk, a));
        A2Callback c3(&a, &A::add);
    
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
            printf("   because [%d] != 2 * [%d]\n", result.y, result.x);            }
}
