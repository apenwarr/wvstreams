/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvlog.h"
#include "wvlogbuffer.h"


WVTEST_MAIN("extremely basic test")
{
    // this test just basically helps see if there's an fd or memory leak
    // caused by creating/using a wvlog...
    WvLog log("logtest", WvLog::Info);
    log("Hello!\n");
    WVPASS(log.isok());
}


WVTEST_MAIN("line-breaking logic")
{
    WvLogBuffer logbuffer(10);
    WvLog log1("log", WvLog::Info);
    WvLog log2(log1.split(WvLog::Debug));
    WvLog log3("other log", WvLog::Info);

    log1("first part");
    log2("second part");
    log1("third part");
    log3("fourth part");
    log2("Message: ");
    log1(WvLog::Debug, "this is the message\n");

    WvLogBuffer::MsgList::Iter i(logbuffer.messages());
    i.rewind();

    WVPASS(i.next());
    WVPASSEQ(i->level, WvLog::Info);
    WVPASSEQ(i->source, "log");
    WVPASSEQ(i->message, "first part");

    WVPASS(i.next());
    WVPASSEQ(i->level, WvLog::Debug);
    WVPASSEQ(i->source, "log");
    WVPASSEQ(i->message, "second part");

    WVPASS(i.next());
    WVPASSEQ(i->level, WvLog::Info);
    WVPASSEQ(i->source, "log");
    WVPASSEQ(i->message, "third part");

    WVPASS(i.next());
    WVPASSEQ(i->level, WvLog::Info);
    WVPASSEQ(i->source, "other log");
    WVPASSEQ(i->message, "fourth part");

    WVPASS(i.next());
    WVPASSEQ(i->level, WvLog::Debug);
    WVPASSEQ(i->source, "log");
    WVPASSEQ(i->message, "Message: this is the message");

    WVFAIL(i.next());
}

