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


WVTEST_MAIN("iterators")
{
    UniConfRoot root("temp:");
    root["1"].set("foo");
    root["2"].set("blah");
    root["2/2b"].set("sub");
    
    WVPASSEQ(itcount(root), 2);
    WVPASSEQ(itcount(root["2"]), 1);
    WVPASSEQ(itcount(root["1"]), 0);
    WVPASSEQ(itcount(root["2/2b"]), 0);
    WVPASSEQ(itcount(root["3"]), 0);
}
