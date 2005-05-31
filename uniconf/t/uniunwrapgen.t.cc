#include "uniunwrapgen.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "wvtest.h"

static int itcount(UniConfGen::Iter *i)
{
    int count = 0;
    for (i->rewind(); i->next(); )
	count++;
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
