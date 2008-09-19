#include "wvistreamlist.h"
#include "wvtest.h"
#include "wvloopback.h"
#include "wvtimeutils.h"
#ifdef _WIN32
#include "streams.h"
#endif

static void cb(int *x)
{
    (*x)++;
}


WVTEST_MAIN("spinning list")
{
    int scount = 0, lcount = 0;
    
    WvStream s;
    s.setcallback(wv::bind(cb, &scount));
    
    WvIStreamList l;
    l.setcallback(wv::bind(cb, &lcount));
    l.append(&s, false, "stream");
    
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
    s.setcallback(wv::bind(cb, &scount));
    
    WvIStreamList l;
    l.setcallback(wv::bind(cb, &lcount));
    l.append(&s, false, "stream");
    
    s.runonce(0);
    WVPASSEQ(scount, 0);
    WVPASSEQ(lcount, 0);
    
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
    wvdelay(100);
    l.runonce(5000);
    s.drain();
    WVPASSEQ(scount, 1);
    WVPASSEQ(lcount, 1);
    
    scount = lcount = 0;
    l.runonce(0);
    WVPASSEQ(scount, 0);
    WVPASSEQ(lcount, 0);
}
