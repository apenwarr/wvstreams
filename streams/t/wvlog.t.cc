/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvlog.h"
#include "wvlogbuffer.h"
#include "wvlogfile.h"
#include "wvfileutils.h"


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

WVTEST_MAIN("timestamps")
{
    WvString logfilename = wvtmpfilename("wvlog-timestamps");
    WvLogFileBase logfile(logfilename, WvLog::Debug);
    WvLog log(__FUNCTION__, WvLog::Debug);
    time_t first_time = time(NULL);
    log("First message\n");
    while (time(NULL) - first_time < 2)
        sleep(1);
    log("Second message\n");
    logfile.close();

    WvFile file(logfilename, O_RDONLY);
    WVPASS(file.isok());
    // 0123456789012345678901
    // Nov 10 10:13:15 GMT-4: 
    WvString first_timestamp = file.getline();
    first_timestamp.edit()[21] = '\0';
    wvout->print("first timestamp: %s\n", first_timestamp);    
    WvString second_timestamp = file.getline();
    second_timestamp.edit()[21] = '\0';
    wvout->print("second timestamp: %s\n", second_timestamp);    
    WVFAILEQ(first_timestamp, second_timestamp);

    WVPASS(unlink(logfilename) == 0);
}

#if 0
WVTEST_MAIN("wvlog performance")
{
    WvString logfilename = wvtmpfilename("wvlog-timestamps");
    WvLogFileBase logfile(logfilename, WvLog::Debug);
    WvLog log(__FUNCTION__, WvLog::Debug);
    
    time_t start;
    start = time(NULL);
    while (time(NULL) == start)
        usleep(1000);
    start = time(NULL);

    int count = 0;
    while (time(NULL) - start < 10)
        log("Message %s\n", count++);

    wvout->print("Total %s log messages\n", count);

    WVPASS(unlink(logfilename) == 0);
}
#endif
