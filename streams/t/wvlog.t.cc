/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvlog.h"

WVTEST_MAIN("extremely basic test")
{
    // this test just basically helps see if there's an fd or memory leak
    // caused by creating/using a wvlog...
    WvLog log("logtest", WvLog::Info);
    log("Hello!\n");
    WVPASS(log.isok());
}
