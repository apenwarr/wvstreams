#include "uniunwrapgen.h"
#include "uniconfroot.h"
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
