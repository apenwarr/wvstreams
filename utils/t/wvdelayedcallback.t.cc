#include "wvdelayedcallback.h"
#include "wvistreamlist.h"
#include "wvtest.h"

static int num;

static void add_cb(int howmuch)
{
    num += howmuch;
}

typedef wv::function<void(int)> InnerCallback;

WVTEST_MAIN("repetition")
{
    num = 0;

    WvIStreamList l;
    
    // Create all these with different syntax to make sure all the different
    // syntaxes actually work.
    InnerCallback xx = &add_cb;
    InnerCallback cb1 = WvDelayedCallback<InnerCallback>(xx);
    InnerCallback cb2 = wv::delayed(xx);
    wv::delayed(&add_cb);
    InnerCallback cb3 = wv::delayed<InnerCallback>(wv::bind(&add_cb, _1));
    InnerCallback cb4 = wv::delayed(&add_cb);
    InnerCallback cb5 = wv::delayed(wv::delayed(wv::delayed(&add_cb)));
    
    l.runonce(10);
    WVPASSEQ(num, 0);
    
    cb1(5);
    l.runonce(10);
    WVPASSEQ(num, 5);
    
    l.runonce(10);
    WVPASSEQ(num, 5);
    
    cb1(1);
    cb1(2);
    WVPASSEQ(num, 5);
    l.runonce(10);
    WVPASSEQ(num, 7);
    
    l.runonce(10);
    WVPASSEQ(num, 7);
    
    cb1(1);
    cb1 = 0;  // though the callback is now gone, the underlying stream is still
	      // on the globallist (though close()d), and alarm(0)'d; it will
	      // still run.
    l.runonce(10);
    WVPASSEQ(num, 8);

#if WVDELAYED_SUPPORTS_NESTING_PROPERLY
    // FIXME: See wvdelayedcallback.h for information about why nested
    // WvDelayedCallback objects don't actually work.
    cb5(100);
    cb4(200);
    WVPASSEQ(num, 7);
    l.runonce(10);
    WVPASSEQ(num, 207);
    l.runonce(10);
    WVPASSEQ(num, 207);
    l.runonce(10);
    WVPASSEQ(num, 207);
    l.runonce(10);
    WVPASSEQ(num, 307);
#endif    
}

