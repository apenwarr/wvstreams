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

    bool check(WvString test, WvString expected)
    {
        WvString value(uniconf[test].get()); 
        if (value == expected)
        {
            wvcon->print("OK - %s: got \"%s\"\n", test, value);
            return true;
        }
        else
        {
            wvcon->print("FAIL - %s: expected \"%s\", got \"%s\"\n",
                    test, expected, value);
            return false;
        }
    }

    void run_test()
    {
        bool pass = check("chickens/bob", "goof");
        if (pass) pass = check("users/apenwarr/ftp", "1");
        if (!pass)
        {
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
