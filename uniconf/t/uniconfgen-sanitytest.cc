/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2006 Net Integration Technologies, Inc.
 * 
 * Basic sanity tests that all UniConf generators should probably pass
 */ 
#include "uniconfgen-sanitytest.h"

#include "uniconf.h"
#include "uniconfdaemon.h"
#include "uniconfgen.h"
#include "uniconfroot.h"
#include "uniwatch.h"
#include "wvfork.h"
#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvtest.h"

#include <signal.h>
#include <time.h>
#include <sys/types.h>

#ifndef _WIN32
#include <sys/wait.h>
#endif

void UniConfTestDaemon::boring_server_cb(WvStringParm sockname, 
        WvStringParm server_moniker)
{
    {
        wverr->close();
        time_t start = time(NULL);

        UniConfRoot uniconf(server_moniker);
        UniConfDaemon daemon(uniconf, false, NULL);

        unlink(sockname);
        daemon.setupunixsocket(sockname);

        WvIStreamList::globallist.append(&daemon, false, "uniconfd");
        // Make sure to commit suicide after half an hour, just in case
        while (time(NULL) < start + 30*60)
        {
            WvIStreamList::globallist.runonce();
            // usleep(1000); // should not be necessary
        }
    }
    _exit(0);
}


void UniConfTestDaemon::autoinc_server_cb(WvStringParm sockname, 
        WvStringParm server_moniker)
{
    {
        wverr->close();
        time_t start = time(NULL);

        UniConfRoot uniconf(server_moniker);
        UniConfDaemon daemon(uniconf, false, NULL);

        unlink(sockname);
        daemon.setupunixsocket(sockname);

        WvIStreamList::globallist.append(&daemon, false, "uniconfd");
        // Make sure to commit suicide after half an hour, just in case
        while (time(NULL) < start + 30*60)
        {
            uniconf.setmeint(uniconf.getmeint()+1);
            WvIStreamList::globallist.runonce();
            // usleep(1000); // should not be necessary
        }
    }
    _exit(0);
}


UniConfTestDaemon::UniConfTestDaemon(WvStringParm _sockname, 
        WvStringParm _server_moniker, 
        UniConfDaemonServerCb server_cb) :
    sockname(_sockname),
    server_moniker(_server_moniker)
{
    pid_t child = wvfork();
    if (child == 0)
        server_cb(sockname, server_moniker);
    WVPASS(child > 0);

    server_pid = child;

    return;
}

UniConfTestDaemon::~UniConfTestDaemon()
{
#ifndef _WIN32
    // Never, ever, try to kill pids -1 or 0.
    if (server_pid <= 0)
        fprintf(stderr, "Refusing to kill pid %i.\n", (int)server_pid);
    else
        kill(server_pid, 15);

    int status;
    pid_t rv;
    while ((rv = waitpid(server_pid, &status, 0)) != server_pid)
    {
        // in case a signal is in the process of being delivered...
        if (rv == -1 && errno != EINTR)
            break;
    }
    WVPASSEQ(rv, server_pid);
    WVPASS(WIFSIGNALED(status));
#endif
    
    unlink(sockname);
}

void UniConfGenSanityTester::clear_generator(IUniConfGen *g)
{
    WvLog log("clear_generator");
    UniConfGen::Iter *ii = g->iterator("/");
    if (ii)
    {
        for (ii->rewind(); ii->next(); )
        {
            log("removing %s (val %s)\n", ii->key(), ii->value());
            g->set(ii->key(), WvString::null);
        }
        delete ii;
    }
}


void UniConfGenSanityTester::sanity_test(IUniConfGen *g, WvStringParm moniker)
{
    test_haschildren_gen(g);
    test_haschildren_moniker(moniker);
    test_trailing_slashes(g, moniker);
    test_iter_sanity(moniker);
    test_recursive_iter_sanity(moniker);
}

void UniConfGenSanityTester::test_haschildren_gen(IUniConfGen *g)
{
    clear_generator(g);

    WVFAIL(g->haschildren("/"));
    
    g->set("/", "blah");
    g->commit();
    WVFAIL(g->haschildren("/"));
    
    g->set("/x", "pah");
    g->commit();
    WVPASS(g->haschildren("/"));
    WVFAIL(g->haschildren("/x"));

    // Setting a section to an empty string is different from deleting it.
    g->set("/", WvString(""));
    WVPASS(g->haschildren("/"));
    WVFAIL(g->haschildren("/x"));
    
    g->set("/", WvString::null);
    g->commit();
    WVFAIL(g->haschildren("/"));

    clear_generator(g);
}

// FIXME: This tests different things from the non-moniker test. They should
// be combined, accept a regular UniConf as a parameter, and let the caller
// worry about how to make one.
void UniConfGenSanityTester::test_haschildren_moniker(WvStringParm moniker)
{
    if (!moniker)
        return;

    // Checking notifications.. (we will assume that we are getting the
    // right keys for now)
    UniConfGenSanityTester::CbCounter notifywatcher;

    UniConfRoot cfg(moniker);
    UniWatch watcher(cfg["/"],
		     wv::bind(&UniConfGenSanityTester::CbCounter::callback,
			      &notifywatcher, _1, _2));
    WVFAIL(cfg.haschildren());

    WVPASSEQ(cfg.getme(), WvString::empty);
    WVPASS(cfg.exists());
    WVFAIL(cfg["x"].exists());

    notifywatcher.cbs = 0;
    
    cfg.setme("blah");
    cfg.commit();
    WVFAIL(cfg.haschildren());
    WVPASS(cfg.exists());
    // Note: We might get more than one notification for the change, e.g. if
    // we are using a UniListGen.
    WVPASSLT(0, notifywatcher.cbs);
    int old_cbs = notifywatcher.cbs;
    
    cfg["x"].setme("pah");
    cfg.commit();
    WVPASS(cfg["x"].exists());
    WVPASS(cfg.haschildren());
    WVFAIL(cfg["x"].haschildren());
    WVPASSLT(old_cbs, notifywatcher.cbs);

    // Don't send notifications if the key doesn't change
    old_cbs = notifywatcher.cbs;
    cfg["x"].setme("pah");
    cfg.commit();
    WVPASSEQ(notifywatcher.cbs, old_cbs);
    
    cfg.remove();
    cfg.commit();
    WVFAIL(cfg.haschildren());
    // We should get notifications for both /x and / being deleted
    WVPASSLT(old_cbs + 1, notifywatcher.cbs);

    // FIXME: UniIniGen fails this test.  See BUGZID:22439
#if 0
    // Don't send notifications if the key doesn't change
    old_cbs = notifywatcher.cbs;
    cfg.remove();
    cfg.commit();
    WVPASSEQ(notifywatcher.cbs, old_cbs);

    WVFAIL(cfg.exists());
    WVPASSEQ(cfg.getme(), WvString::null);
    WVFAIL(cfg["x"].exists());
    WVPASSEQ(cfg["x"].getme(), WvString::null);
#endif
}

void UniConfGenSanityTester::test_trailing_slashes(IUniConfGen *g, 
        WvStringParm moniker)
{
    clear_generator(g);

    g->set("", "xyzzy");
    g->commit();
    WVPASSEQ(g->get("/"), "xyzzy");
    WVPASSEQ(g->get("///"), "xyzzy");
    WVPASSEQ(g->get(""), "xyzzy");
    WVPASSEQ(g->get("//"), "xyzzy");

    WVPASSEQ(g->get("///simon"), WvString::null);
    WVPASSEQ(g->get("///simon/"), WvString::null);
    WVPASSEQ(g->get("/simon///"), WvString::null);

    g->set("//simon", "law");
    g->commit();
    WVPASSEQ(g->get("/Simon"), "law");
    WVPASSEQ(g->get("simon///"), WvString::null);

    g->set("//simon/", "LAW");
    g->commit();
    WVPASSEQ(g->get("/siMON"), "law");
    WVPASSEQ(g->get("simon///"), WvString::null);
    WVFAIL(g->haschildren("simon//"));

    g->set("//simon/law", "1");
    g->commit();
    WVPASSEQ(g->get("/simon/law"), "1");
    g->set("//simon/", WvString::null);
    g->commit();
    WVPASSEQ(g->get("/simon/law"), WvString::null);
    WVFAIL(g->haschildren("simon///"));

    // Test that we delete immediate and recursive children when we delete a
    // section
    g->set("//simon/law", "1");
    g->set("//simon/law/foo", "1");
    g->set("//simon/law/foo/bar", "1");
    g->commit();
    WVPASSEQ(g->get("/simon/law"), "1");
    WVPASSEQ(g->get("/simon/law/foo"), "1");
    WVPASSEQ(g->get("/simon/law/foo/bar"), "1");
    g->set("//simon/", WvString::null);
    g->commit();
    WVPASSEQ(g->get("/simon/law"), WvString::null);
    WVPASSEQ(g->get("/simon/law/foo"), WvString::null);
    WVPASSEQ(g->get("/simon/law/foo/bar"), WvString::null);
    WVFAIL(g->haschildren("simon///law//foo"));
    WVFAIL(g->haschildren("simon///law//"));
    WVFAIL(g->haschildren("simon///"));

    // Test that we delete immediate and recursive autovivified children when
    // we delete a section
    g->set("//simon/law/foo/bar", "1");
    g->commit();
    WVPASSEQ(g->get("/simon/law"), WvString::null);
    WVPASSEQ(g->get("/simon/law/foo"), WvString::null);
    WVPASSEQ(g->get("/simon/law/foo/bar"), "1");
    g->set("//simon/", WvString::null);
    g->commit();
    WVPASSEQ(g->get("/simon/law"), WvString::null);
    WVPASSEQ(g->get("/simon/law/foo"), WvString::null);
    WVPASSEQ(g->get("/simon/law/foo/bar"), WvString::null);
    WVFAIL(g->haschildren("simon///law//foo/"));
    WVFAIL(g->haschildren("simon///law//"));
    WVFAIL(g->haschildren("simon///"));

    if (!!moniker)
    {
        UniConfRoot cfg(moniker);
        cfg.setme("xyzzy");
        cfg.commit();
        WVPASSEQ(cfg.xget("/"), "xyzzy");
        WVPASSEQ(cfg.xget("///"), "xyzzy");
        WVPASSEQ(cfg[""].getme(), "xyzzy");
        WVPASSEQ(cfg[""][""]["/"].xget("/"), "xyzzy");

        WVPASSEQ(cfg[""][""]["/"]["simon"].xget(""), WvString::null);
        WVPASSEQ(cfg[""][""]["/"]["simon"].xget("///"), WvString::null);

        cfg[""]["/"][""]["simon"].setme("law");
        cfg.commit();
        WVPASSEQ(cfg[""][""]["/"].xget("simon"), "law");
        WVPASSEQ(cfg[""][""]["/"]["simon"].getme(), "law");
        WVPASSEQ(cfg[""][""]["/"]["simon"].xget(""), WvString::null);
        WVPASSEQ(cfg[""][""]["/"]["simon"].xget("///"), WvString::null);

        cfg[""]["/"][""]["simon"].xset("/", "LAW");
        cfg.commit();
        WVPASSEQ(cfg[""][""]["/"].xget("simon"), "law");
        WVPASSEQ(cfg[""][""]["/"].xget("simon/"), "");
        WVPASSEQ(cfg[""][""]["/"]["simon"].getme(), "law");
        WVPASSEQ(cfg[""][""]["/"]["simon"].xget(""), WvString::null);
        WVPASSEQ(cfg[""][""]["/"]["simon"].xget("///"), WvString::null);
        WVFAIL(cfg[""]["simon"][""].haschildren());
        WVFAIL(cfg["simon"]["/"].haschildren());

        cfg[""][""]["/"]["simon"].xset("/law", "1");
        cfg.commit();
        WVPASSEQ(cfg[""][""]["/"]["simon"][""].xget("/law"), "1");
        cfg[""][""]["/"]["simon"]["law"].xset("", "2");
        cfg.commit();
        WVPASSEQ(cfg[""][""]["/"]["simon"][""].xget("/law"), "1");
        cfg[""][""]["/"]["simon"]["law"].xset("/", "3");
        cfg.commit();
        WVPASSEQ(cfg[""][""]["/"]["simon"][""].xget("/law"), "1");
        cfg[""]["/"][""]["simon"].xset("/", "LAW");
        cfg.commit();
        WVPASSEQ(cfg[""][""]["/"].xget("simon"), "law");
        WVPASSEQ(cfg[""][""]["/"].xget("simon/"), "");
        cfg[""][""]["/"]["simon"][""].remove();
        cfg.commit();
        WVPASSEQ(cfg[""][""]["/"]["simon"][""].xget("/law"), WvString::null);
        WVFAIL(cfg[""]["simon"]["/"].haschildren());

        WVPASSEQ(cfg[""].getme(), "xyzzy");
    }

    clear_generator(g);
}


// FIXME: Won't run for generators that have no moniker
void UniConfGenSanityTester::test_iter_sanity(WvStringParm moniker)
{
    if (!moniker)
        return;

    UniConfRoot root(moniker);
    root.remove();
    root.commit();

    root.xset("Foo/0", "Bar");
    root.xset("Foo/1", "Baz");
    root.xset("Foo/1/a", "Baz");
    root.xset("Foo/2", "Baz");
    root.xset("Bar/q", "Baz");

    UniConf::Iter ii(root["Foo"]);
    int jj;
    for (jj = 0, ii.rewind(); ii.next(); jj++)
    {
        WVPASSEQ(ii->key().printable(), WvString(jj));
    }
    // Check that we only iterated over three things
    WVPASSEQ(jj, 3);
}

// FIXME: Won't run for generators that have no moniker
void UniConfGenSanityTester::test_recursive_iter_sanity(WvStringParm moniker)
{
    if (!moniker)
        return;

    UniConfRoot root(moniker);
    root.remove();
    root.commit();

    root.xset("Foo/a", "1");
    root.xset("Foo/b", "2");
    root.xset("Foo/b/c", "3");
    root.xset("Foo/d/e", "5");
    root.xset("Bar/q", "Baz");

    UniConf::RecursiveIter ii(root["/Foo"]);
    int jj = 0;
    for (ii.rewind(); ii.next(); )
    {
        jj++;
        int val = ii->getmeint();
        if (ii->fullkey().printable() == "Foo/d")
        {
            WVPASSEQ(val, 0);
            // The fourth value we read is Foo/d, which is autovivified
            val = 4;
        }

        WVFAILEQ(ii->fullkey().printable(), ii->getme());
        WVPASSEQ(val, jj);
        //WVPASSEQ(ii->fullkey().printable(), ii->getme());
    }
    // Check that we iterated over the four entries under Foo, as well as the
    // auto-vivified Foo/d
    WVPASSEQ(jj, 5);
}

