#include "wvistreamlist.h"
#include "wvtest.h"
#include "wvloopback.h"

static void cb(WvStream &s, void *userdata)
{
    int *x = (int *)userdata;
    (*x)++;
}


WVTEST_MAIN("spinning list")
{
    int scount = 0, lcount = 0;
    
    WvStream s;
    s.setcallback(cb, &scount);
    
    WvIStreamList l;
    l.setcallback(cb, &lcount);
    l.append(&s, false);
    
    l.alarm(0);
    l.runonce(0);
    WVPASSEQ(scount, 0);
    WVPASSEQ(lcount, 1);
    
    scount = lcount = 0;
    s.alarm(0);
    l.runonce(0);
    WVPASSEQ(scount, 1);
    WVPASSEQ(lcount, 1);
    
    scount = lcount = 0;
    s.alarm(0);
    l.alarm(0);
    l.runonce(0);
    WVPASSEQ(scount, 1);
    WVPASSEQ(lcount, 1);
    
    scount = lcount = 0;
    l.runonce(0);
    WVPASSEQ(scount, 0);
    WVPASSEQ(lcount, 0);
}


WVTEST_MAIN("spinning list 2")
{
    int scount = 0, lcount = 0;
    
    WvLoopback s;
    s.setcallback(cb, &scount);
    
    WvIStreamList l;
    l.setcallback(cb, &lcount);
    l.append(&s, false);
    
    l.alarm(0);
    l.runonce(0);
    WVPASSEQ(scount, 0);
    WVPASSEQ(lcount, 1);
    
    scount = lcount = 0;
    s.write("x");
    l.runonce(5000);
    s.drain();
    WVPASSEQ(scount, 1);
    WVPASSEQ(lcount, 1);
    
    scount = lcount = 0;
    s.write("x");
    l.alarm(0);
    sleep(1);
    l.runonce(5000);
    s.drain();
    WVPASSEQ(scount, 1);
    WVPASSEQ(lcount, 1);
    
    scount = lcount = 0;
    l.runonce(0);
    WVPASSEQ(scount, 0);
    WVPASSEQ(lcount, 0);
}
