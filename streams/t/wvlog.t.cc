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


WVTEST_MAIN("log copying 1")
{
    WvLog *l1 = new WvLog("foo");
    WVPASS(l1->isok());
    WvLog *l2 = new WvLog("blah");
    WVPASS(l2->isok());
    *l2 = *l1;
    WVPASS(l2->isok());
    l1->release();
    WVPASS(l2->isok());
    WvLog *l3 = new WvLog("weasels");
    WVPASS(l3->isok());
    *l3 = *l2;
    WVPASS(l3->isok());
    l2->release();
    l3->print("yak!");
    l3->release();
}

WVTEST_MAIN("log copying 2")
{
    WvLog *l1 = new WvLog("foo");
    WVPASS(l1->isok());
    WvLog *l2 = new WvLog(*l1);
    WVPASS(l2->isok());
    l1->release();
    WvLog *l3 = new WvLog(*l2);
    l3->print("yak!");
    l2->release();
    l3->print("yak2!");
    l3->release();
}
