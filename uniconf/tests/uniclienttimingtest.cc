// Test client for the uniconf daemon

/**
 * FIXME: This test case is not as useful at it could be because:
 *   1) it only tests a very small subset of features
 *   2) it does not setup / teardown its environment explicitly
 */

#include "wvstream.h"
#include "uniconf.h"
#include "wvlogrcv.h"

int alarms[] = { 10, 100, 1000, 10000, 10000, 10000, -1 };

class TestStream : public WvStream
{
public:
    TestStream(const UniConf &_uniconf) :
        WvStream(),
        uniconf(_uniconf),
        stage(0),
        done(false),
        passed(true)
    {
        alarm(0);
    }

    void run_test()
    {
        WvString test("chickens/bob");
        WvString expected("goof");
        WvString value(uniconf[test].get());
        if (value == expected)
        {
            wvcon->print("OK - %s: got \"%s\"\n", test, value);
        }
        else
        {
            wvcon->print("FAIL - %s: expected \"%s\", got \"%s\"\n",
                    test, expected, value);
            passed = false;
            done = true;
        }
    }
        
    virtual void execute()
    {
        WvStream::execute();
        if (alarm_was_ticking)
        {
            run_test();
            if (alarms[stage] == -1)
                done = true;
            else
                alarm(alarms[stage]);
            stage++;

        }
    }

    virtual bool isok() const
    {
        return !done;
    }
    
public:
    const UniConf uniconf;
    unsigned int stage;
    bool done;
    bool passed;
};

void usage()
{
    wvcon->print("uniconfclienttest usage:\n");
    wvcon->print("uniconfclienttest [-h]\n");
    wvcon->print("\t-h - display this message\n");
    exit(0);
}

int main(int argc, char **argv)
{
    WvString location("tcp:localhost:4111");

    if (argc == 2 && !strcasecmp(argv[1], "-h"))
    {
        usage();
    }

    WvLogConsole clog(1, WvLog::Debug5);

    UniConfRoot root(location);
    TestStream t(root);
    
    while (t.isok())
        if (t.select(-1)) t.callback();     

    if (t.passed)
        wvcon->print("\n***** PASSED *****\n\n");
    else
        wvcon->print("\n///// FAILED /////\n\n");
}
