#include "wvdelayedcallback.h"
#include "wvistreamlist.h"
#include "wvtest.h"

static int num;

static void add_cb(int howmuch)
{
    num += howmuch;
}

typedef WvCallback<void, int> InnerCallback;

WVTEST_MAIN("repetition")
{
    num = 0;

    WvIStreamList l;
    WvCallback<void,int> cb1 = WvDelayedCallback<WvCallback<void,int> >(&add_cb);
    
    l.runonce(0);
    WVPASSEQ(num, 0);
    cb1(5);
    l.runonce(0);
    WVPASSEQ(num, 5);
    l.runonce(0);
    WVPASSEQ(num, 5);
    cb1(1);
    cb1(2);
    WVPASSEQ(num, 5);
    l.runonce(0);
    WVPASSEQ(num, 7);
    l.runonce(0);
    WVPASSEQ(num, 7);
    cb1(1);
    cb1 = 0;
    l.runonce(0);
    WVPASSEQ(num, 7);
}
