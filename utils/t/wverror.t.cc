#include "wvtest.h"
#include "wverror.h"
#include "wvstring.h"

WVTEST_MAIN("Basic error test")
{
    WvError testerr;
    testerr.set("This is a special error.");
    WVPASS(testerr.str() == "This is a special error.");
    
    testerr.set(0);     // Should not change the error.
    WVFAIL(testerr.str() == "Success");

    testerr.reset();    // Actually resetting the error to default.
    WVPASS(testerr.str() == "Success");

    WvString prev(testerr.str());

    // Just going through first 10 error codes..
    // for some reason.. or no reason at all.. =/
    for (int i=1; i<10; i++)
    {
        testerr.set(i);
        WVFAIL(prev == testerr.str()); 
        prev = testerr.str();
        testerr.reset();
    }
    
}
