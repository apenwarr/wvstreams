#include "wvtest.h"
#include "unicallbackgen.h"

WvString generator(const UniConfKey &key)
{
    static int i = 0;
    return i++;
}

WVTEST_MAIN("even more basic")
{
    UniCallbackGen g(1);
    WVPASS(!g.get("key"));
    g.setvaluegencallback("key", UniCallbackGenCallback(generator));
    WVPASS(g.get("key") == WvString(0));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(1));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(2));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(3));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(4));
    g.setvaluegencallback("key", UniCallbackGenCallback());
    WVPASS(!g.get("key"));
}
