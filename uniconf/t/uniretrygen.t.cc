#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "uniretrygen.h"
#include "uniclientgen.h"
#include "uniconfgen-sanitytest.h"
#include "wvfileutils.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


struct UniRetryGenTester
{
    WvString socket;
    WvString ini;
    pid_t uniconfd_pid;
    UniRetryGenTester() :
        socket("/tmp/uniretrygen-uniconfd-%s", getpid()),
        ini("/tmp/uniretrygen-uniconfd.ini-%s", getpid()),
        uniconfd_pid(0)
    { }
};


void wait_for_daemon(UniRetryGenTester t)
{
    UniConfRoot another_cfg(WvString("retry:unix:%s 100", t.socket));
    
    for (;;)
    {
	another_cfg.xset("wait", "pong");
	if (another_cfg.xget("wait") == "pong") break;
	fprintf(stderr, "still waiting for connection.\n");
	wvdelay(100);
    }

    fprintf(stderr, "managed to connect\n");
}


WVTEST_MAIN("UniRetryGen Sanity Test")
{
    UniRetryGen *gen = new UniRetryGen("temp:");
    UniConfGenSanityTester::sanity_test(gen, "retry:temp: 100");
    WVRELEASE(gen);
}

WVTEST_MAIN("UniRetryGen: uniconfd")
{
    signal(SIGPIPE, SIG_IGN);

    UniRetryGenTester t;
    
    unlink(t.ini);
    
    UniConfRoot cfg(WvString("retry:unix:%s 100", t.socket));
    cfg["/key"].setme("value");
    WVPASS(!cfg["/key"].exists());

    {
        UniConfTestDaemon daemon(t.socket, WvString("ini:%s", t.ini));
        wait_for_daemon(t);

	wvdelay(100); // guarantee that the retry interval has expired
        cfg["/key"].setme("value");
        WVPASSEQ(cfg["/key"].getme(), "value");
        
        cfg.commit();
    }
    
    // no delay necessary: we disconnect right away
    WVPASS(!cfg["/key"].exists());
    
    {
        UniConfTestDaemon daemon(t.socket, WvString("ini:%s", t.ini));
        wait_for_daemon(t);
        
	wvdelay(100); // guarantee that the retry interval has expired
        WVPASSEQ(cfg["/key"].getme(), "value");
        
        cfg.commit();
    }

    WVPASS(!cfg["/key"].exists());
}

bool reconnected = false;
void reconnect_cb(UniRetryGen &uni)
    { reconnected = true; }

WVTEST_MAIN("UniRetryGen: reconnect callback")
{
    signal(SIGPIPE, SIG_IGN);
    UniRetryGenTester t;

    unlink(t.ini);
    
    UniConfRoot cfg;
    cfg.mountgen(new UniRetryGen(WvString("unix:%s", t.socket),
                UniRetryGen::ReconnectCallback(reconnect_cb), 100));

    reconnected = false;

    UniConfTestDaemon daemon(t.socket, WvString("ini:%s", t.ini));
    wait_for_daemon(t);
    
    wvdelay(100); // guarantee that the retry interval has expired
    cfg.getme(); // Do something to reconnect
    WVPASS(reconnected);
}

WVTEST_MAIN("UniRetryGen: immediate reconnect")
{
    signal(SIGPIPE, SIG_IGN);

    UniRetryGenTester t;
    
    unlink(t.ini);

    // Need to set the reconnect delay to 0 to read immediately
    UniConfRoot cfg(WvString("retry:unix:%s 0", t.socket));

    {
        UniConfTestDaemon daemon(t.socket, WvString("ini:%s", t.ini));
        wait_for_daemon(t);

        cfg["/key"].setme("value");
        WVPASSEQ(cfg["/key"].getme(), "value");
        
        cfg.commit();
    }
    
    // don't check anything before restarting so cfg doesn't know that
    // uniconfd has disconnected.
    {
        UniConfTestDaemon daemon(t.socket, WvString("ini:%s", t.ini));
        wait_for_daemon(t);

        cfg.getme(); // Do something to reconnect
        WVPASSEQ(cfg["/key"].getme(), "value");
    }

    WVPASS(!cfg["/key"].exists());
}


WVTEST_MAIN("UniRetryGen: mount point exists")
{
    // bug 9769
    UniConfRoot uniconf("temp:");
    
    WVPASS(uniconf["foo"].mount("retry:unix:/tmp/foobar 100"));

    WVPASS(uniconf["foo"].exists());
    WVPASSEQ(uniconf["foo"].xget("", WvString::null), "");

    WVFAIL(uniconf["foo/bar"].exists());
    WVPASSEQ(uniconf["foo"].xget(""), WvString::null);    
}

