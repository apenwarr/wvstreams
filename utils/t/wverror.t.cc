#include "wvtest.h"
#include "wverror.h"


void testnoerr(const WvErrorBase &e)
{
    WVPASS(e.isok());
    WVPASS(e.geterr() == 0);
    WVPASSEQ(e.errstr(), strerror(0));
}


void testerr(const WvErrorBase &e, int err)
{
    WVPASS(!e.isok());
    WVPASS(e.geterr() == err);
    WVPASSEQ(e.errstr(), strerror(err));
}


void testerrstr(const WvErrorBase &e, WvStringParm err)
{
    WVPASS(!e.isok());
    WVPASS(e.geterr() == -1);
    WVPASSEQ(e.errstr(), err);
}


WVTEST_MAIN("simple set")
{
    WvError e;

    // initial state
    testnoerr(e);

    // set an int
    e.seterr(1);
    testerr(e, 1);

    // int not overwritten
    e.seterr(2);
    testerr(e, 1);

    e.seterr("special");
    testerr(e, 1);

    // reset to initial
    e.noerr();
    testnoerr(e);

    // set a string
    e.seterr("special");
    testerrstr(e, "special");

    // string not overwritten
    e.seterr(2);
    testerrstr(e, "special");

    e.seterr("specialer");
    testerrstr(e, "special");
}

WVTEST_MAIN("copy WvErrorBase")
{
    WvError e1;
    WvError e2;
   
    // initial over empty 
    e1.seterr(e2);
    testnoerr(e1);

    // initial over int
    e1.seterr(1);
    e1.seterr(e2);
    testerr(e1, 1);
    e1.noerr();
    
    // initial over str
    e1.seterr("special");
    e1.seterr(e2);
    testerrstr(e1, "special");
    e1.noerr();

    e2.seterr(2);
    
    // int over empty 
    e1.seterr(e2);
    testerr(e1, 2);
    e1.noerr();
    
    // int over int
    e1.seterr(1);
    e1.seterr(e2);
    testerr(e1, 1);
    e1.noerr();

    // int over str
    e1.seterr("special");
    e1.seterr(e2);
    testerrstr(e1, "special");
    e1.noerr();

    e2.noerr();
    e2.seterr("specialer");

    // str over empty
    e1.seterr(e2);
    testerrstr(e1, "specialer");
    e1.noerr();

    // str over int
    e1.seterr(1);
    e1.seterr(e2);
    testerr(e1, 1);
    e1.noerr();

    // str over str
    e1.seterr("special");
    e1.seterr(e2);
    testerrstr(e1, "special");
    e1.noerr();
}

WVTEST_MAIN("set to -1")
{
    WvError e;

    // overwrite an int
    e.seterr(1);
    e.seterr(-1);
    testerr(e, 1);
    e.noerr();
    
    // overwrite a str
    e.seterr("special");
    e.seterr(-1);
    testerrstr(e, "special");
    e.noerr();

#if 0
    // set directly - should crash
    e.seterr(-1);
    testnoerr(e);
#endif
}
