#include "wvtest.h"
#include "wvloopback2.h"
#include "wvstreamclone.h"

WVTEST_MAIN("loopback2")
{
    IWvStream *_s1, *_s2;
    wv_loopback2(_s1, _s2);
    WvStreamClone s1(_s1), s2(_s2);
    
    WVPASS(s1.isok());
    WVPASS(s2.isok());
    s1.print("s1 output\ns1 line2\n");
    s2.print("s2 output\ns2 line2");
    WVPASSEQ(s1.blocking_getline(1000), "s2 output");
    WVPASSEQ(s2.blocking_getline(1000), "s1 output");
    WVPASSEQ(s2.getline(), "s1 line2");
    WVFAIL(s1.getline()); // no newline found
    s2.nowrite(); // EOF finishes that last line
    WVPASSEQ(s1.blocking_getline(1000), "s2 line2");
}
