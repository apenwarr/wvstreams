#include "uniconfgen-sanitytest.h"

#include "uniconf.h"
#include "uniconfroot.h"
#include "uniconfgen.h"
#include "uniwatch.h"
#include "wvtest.h"
#include "wvlog.h"

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
    UniWatch watcher(cfg["/"], UniConfCallback(&notifywatcher,
                 &UniConfGenSanityTester::CbCounter::callback));
    WVFAIL(cfg.haschildren());
    
    cfg.setme("blah");
    cfg.commit();
    WVFAIL(cfg.haschildren());
    // Note: We might get more than one notification for the change, e.g. if
    // we are using a UniListGen.
    WVPASSLT(0, notifywatcher.cbs);
    
    cfg["x"].setme("pah");
    cfg.commit();
    WVPASS(cfg.haschildren());
    WVFAIL(cfg["x"].haschildren());
    WVPASSLT(1, notifywatcher.cbs);
    
    cfg.remove();
    cfg.commit();
    WVFAIL(cfg.haschildren());
    // We should get notifications for both /x and / being deleted
    WVPASSLT(3, notifywatcher.cbs);
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
    root.xset("Foo/d/e", "4");
    root.xset("Bar/q", "Baz");

    UniConf::RecursiveIter ii(root["/Foo"]);
    int jj;
    for (jj = 0, ii.rewind(); ii.next(); jj++)
    {
        WVFAILEQ(ii->fullkey().printable(), root[ii->fullkey()].getme());
        //WVPASSEQ(root[ii->fullkey()].getmeint(), jj);
    }
    // Check that we iterated over five entries (the four created, plus /Foo/d)
    WVPASSEQ(jj, 5);
}

