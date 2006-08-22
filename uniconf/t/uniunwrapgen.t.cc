#include "uniconfdaemon.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "uniunwrapgen.h"
#include "uniconfgen-sanitytest.h"
#include "wvtest.h"

#include <signal.h>

WVTEST_MAIN("UniUnwrapGen Sanity Test")
{
    UniConfRoot cfg("temp:");
    UniUnwrapGen *gen = new UniUnwrapGen(cfg["/"]);
    UniConfGenSanityTester::sanity_test(gen, "");
    WVRELEASE(gen);
}

static int itcount(UniConfGen::Iter *i)
{
    int count = 0;
    wverr->print("start\n");
    for (i->rewind(); i->next(); )
    {
        wverr->print("visited %s\n", i->key());
	count++;
    }
    wverr->print("end\n");
    delete i;
    return count;
}


WVTEST_MAIN("unwrap basics")
{
    UniConfRoot cfg("temp:");
    cfg.xset("foo/blah/1/2/3", "x1");
    cfg.xset("foo/blah/1/3/3", "x2");
    cfg.xset("boo/fah", "fah string");
    
    cfg["foo2"].mountgen(new UniUnwrapGen(cfg["foo"]), true);
    cfg["foo3"].mountgen(new UniUnwrapGen(cfg["foo2"]), true);
    
    WVPASSEQ(cfg.xget("foo2/blah/1/2/3"), "x1");
    WVPASSEQ(cfg.xget("foo3/blah/1/3/3"), "x2");
    
    UniUnwrapGen g(cfg);
    WVPASSEQ(itcount(g.iterator("/")), 4);
    WVPASSEQ(itcount(g.recursiveiterator("/foo")), 6);
    WVPASSEQ(itcount(g.recursiveiterator("/foo2")), 6);
    WVPASSEQ(itcount(g.recursiveiterator("/foo3")), 6);

    WVPASSEQ(itcount(g.recursiveiterator("/")), 23);

    cfg.xset("foo3/fork", "forky");
    WVPASSEQ(itcount(g.recursiveiterator("/foo")), 7);

    WVPASSEQ(itcount(g.recursiveiterator("/")), 26);
}


WVTEST_MAIN("unwrapgen root")
{
    UniTempGen *temp = new UniTempGen;
    UniConfRoot cfg(temp, true);

    cfg.xsetint("/a/b/c", 5);
    
    WVPASS(temp->exists("/a/b"));
    WVPASS(!temp->get("/a/b").isnull());
    WVFAIL(temp->exists("a/b/"));
    WVFAIL(!temp->get("a/b/").isnull());
    
    WVPASS(cfg["/a/b"].exists());
    WVFAIL(cfg["/a/b/"].exists());
    WVFAIL(cfg["/a/b"][""].exists());
    WVFAIL(!cfg["/a/b"][""].getme().isnull());
    
    UniUnwrapGen *g = new UniUnwrapGen(cfg["a"]);
    WVPASS(g->exists("/"));
    WVPASS(!g->get("/").isnull());
    WVPASS(g->exists(""));
    WVPASS(!g->get("").isnull());
    WVPASS(g->exists("b"));
    WVPASS(!g->get("b").isnull());
    WVFAIL(g->exists("b/"));
    WVFAIL(!g->get("b/").isnull());
    
    UniConfRoot gcfg(g, true);
    WVPASS(gcfg.exists());
//    WVFAIL(gcfg[""].exists());
    WVPASS(gcfg["b"].exists());
    WVFAIL(gcfg["b/"].exists());
    WVFAIL(gcfg["b"][""].exists());
}

WVTEST_MAIN("unwrapgen callbacks")
{
    signal(SIGPIPE, SIG_IGN);

    WvString sockname("/tmp/uniunwrapgen-%s", getpid());

    UniConfTestDaemon daemon(sockname, "temp:");

    printf("Creating a unix: gen\n");
    UniConfRoot cfg(WvString("unix:%s", sockname));
    
    printf("Waiting for daemon to start.\n");
    fflush(stdout);
    int num_tries = 0;
    const int max_tries = 20;
    while (!cfg.isok() && num_tries < max_tries)
    {
        num_tries++;
        WVFAIL(cfg.isok());

        // Try again...
        cfg.unmount(cfg.whichmount(), true);
        cfg.mount(WvString("unix:%s", sockname));
        sleep(1);
    }

    if (WVPASS(cfg.isok()))
        printf("Connected to daemon.\n");
    else
        printf("Connection failed.\n");
    fflush(stdout);

    WVPASSEQ(cfg.xget("a"), WvString::null);
    WVPASSEQ(cfg.xget("a"), WvString::null);
    cfg.xset("a", "foo");
    WVPASSEQ(cfg.xget("a"), "foo");

    printf("Wrapping it in an unwrap: gen\n");
    UniConfRoot unwrap(new UniUnwrapGen(cfg));

    WVPASSEQ(unwrap.xget("a"), "foo");
}

