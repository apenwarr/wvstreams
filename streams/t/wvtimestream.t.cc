#include "wvtest.h"
#include "wvtimestream.h"


#define TIME_INTERVAL 42


WVTEST_MAIN("bug 19953")
{
    WvTimeStream ts;
    IWvStream::SelectInfo si;
    WvTime now;

    FD_ZERO(&si.read);
    FD_ZERO(&si.write);
    FD_ZERO(&si.except);
    si.wants.readable = true;
    si.wants.writable = false;
    si.wants.isexception = false;
    si.max_fd = -1;
    si.msec_timeout = -1;
    si.inherit_request = true;
    si.global_sure = false;

    wvstime_sync();

    ts.set_timer(TIME_INTERVAL);
    WVFAIL(ts.pre_select(si));
    WVFAIL(ts.post_select(si));

    wvstime_set(msecadd(wvstime(), TIME_INTERVAL));
    WVPASS(ts.pre_select(si));
    WVPASS(ts.post_select(si));

    wvstime_set(msecadd(wvstime(), TIME_INTERVAL + 1));
    WVPASS(ts.pre_select(si));
    WVPASS(ts.post_select(si));
    
    ts.set_timer(TIME_INTERVAL + 1);
    wvstime_set(msecadd(wvstime(), TIME_INTERVAL));
    WVFAIL(ts.pre_select(si));
    WVFAIL(ts.post_select(si));
}

