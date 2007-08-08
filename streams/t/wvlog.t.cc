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

WVTEST_MAIN("keep single log lines together")
{
    WvString logfilename = wvtmpfilename("wvlog-together");
    WvLogFileBase logfile(logfilename, WvLog::Debug);
    WvLog log(__FUNCTION__, WvLog::Debug);
    time_t first_time = time(NULL);
    log("First message: ");
    while (time(NULL) - first_time < 2)
        sleep(1);
    log("Second message\n");
    logfile.close();

    WvFile file(logfilename, O_RDONLY);
    WVPASS(file.isok());

    WvString line1 = file.getline();
    WvString line2 = file.getline();

    wvout->print("line 1: %s\n", line1);

    // we want both messages on a single log line
    WVPASS(!line2);
    WVPASS(strstr(line1.cstr(), "First"));
    WVPASS(strstr(line1.cstr(), "Second"));

    WVPASS(unlink(logfilename) == 0);
}

class WvNoisyLogRcv : public WvLogConsole
{
    WvString noise;
    WvLog sublog;
    
public:
    WvNoisyLogRcv(WvStringParm _noise, int fd, WvLog::LogLevel max_level) : 
        WvLogConsole(fd, max_level),
        noise(_noise),
        sublog("WvNoisyLogRcv", max_level)
    {
    }

    void log(WvStringParm source, int _loglevel,
            const char *_buf, size_t len)
    {
        sublog("%s\n", noise);
        return WvLogConsole::log(source, _loglevel, _buf, len);
    }
};

// Test that if a log receiver generates a log message itself while logging,
// that we log a warning about it and don't recurse endlessly.
WVTEST_MAIN("Recursion avoidance")
{
    WvString noise("Recursive noise");
    WvNoisyLogRcv noisy(noise, dup(1), WvLog::Debug5);

    WvString logfilename("/tmp/wvlog-recursive-test.%s", getpid());
    WvLogFileBase logfile(logfilename, WvLog::Debug5);

    WvLog log("Regular log", WvLog::Error);
    WvString logmsg("The pebble that starts an avalanche...");
    log(logmsg);

    WvFile file(logfilename, O_RDONLY);
    WVPASS(file.isok());

    // Test that we received all the log messages we were due
    WVPASS(strstr(file.getline(), "Too many extra log messages "
                "written while writing to the log.  Suppressing "
                "additional messages."));
    for (int i = 0; i < 6; ++i)
        WVPASS(strstr(file.getline(), noise.cstr()));

    WVPASS(strstr(file.getline(), logmsg.cstr()));

    // Cleanup
    WVPASSEQ(unlink(logfilename), 0);
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
