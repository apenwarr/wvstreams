#include "wvtest.h"
#include "uniconfroot.h"

WVTEST_MAIN("no generator")
{
    UniConfRoot root;
}


WVTEST_MAIN("null generator")
{
    UniConfRoot root("null:");
}


static int itcount(const UniConf &cfg)
{
    int count = 0;
    
    UniConf::Iter i(cfg);
    for (i.rewind(); i.next(); )
	count++;
    return count;
}


static int ritcount(const UniConf &cfg)
{
    int count = 0;
    
    UniConf::RecursiveIter i(cfg);
    for (i.rewind(); i.next(); )
    {
	fprintf(stderr, "key: '%s'\n", i->fullkey(cfg).cstr());
	count++;
    }
    return count;
}


WVTEST_MAIN("iterators")
{
    UniConfRoot root("temp:");
    root["1"].set("foo");
    root["2"].set("blah");
    root["2/2b"].set("sub");
    root["x/y/z/a/b/1/2/3"].set("something");
    
    WVPASSEQ(itcount(root), 3);
    WVPASSEQ(itcount(root["2"]), 1);
    WVPASSEQ(itcount(root["1"]), 0);
    WVPASSEQ(itcount(root["2/2b"]), 0);
    WVPASSEQ(itcount(root["3"]), 0);

    WVPASSEQ(ritcount(root), 11);
    WVPASSEQ(ritcount(root["2"]), 1);
    WVPASSEQ(ritcount(root["1"]), 0);
    WVPASSEQ(ritcount(root["2/2b"]), 0);
    WVPASSEQ(ritcount(root["3"]), 0);
    WVPASSEQ(ritcount(root["x/y/z/a/b"]), 3);
    
    UniConf sub(root["x/y/z/a"]);
    UniConf::RecursiveIter i(sub);
    i.rewind(); i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "b");
    i.next();
    WVPASSEQ(i->fullkey(sub).printable(), "b/1");
}
