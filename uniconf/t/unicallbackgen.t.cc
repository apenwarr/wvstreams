#include "wvtest.h"
#include "unicallbackgen.h"

class AutoIncrementer
{
    int value;
    
public:
    
    AutoIncrementer() : value(0) {}
    
    WvString get(const UniConfKey &) { return value++; }
    void set(const UniConfKey &, WvStringParm _value) { value = _value.num(); }
};

WVTEST_MAIN("non-dynamic")
{
    AutoIncrementer ai;
    UniCallbackGen g(1);

    WVPASS(!g.get("key"));
    
    g.setgetcallback("key",
            UniCallbackGenGetCallback(&ai, &AutoIncrementer::get));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(0));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(1));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(2));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(3));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(4));

    g.setgetcallback("key", UniCallbackGenGetCallback());
    WVPASS((g.set("key", "foo"), !g.get("key")));
}

WVTEST_MAIN("dynamic")
{
    AutoIncrementer ai;
    UniCallbackGen g(1);

    WVPASS(!g.get("key"));

    g.update_after_set = false;
    g.update_before_get = true;
    
    g.setgetcallback("key",
            UniCallbackGenGetCallback(&ai, &AutoIncrementer::get));

    WVPASS(g.get("key") == WvString(0));
    WVPASS(g.get("key") == WvString(1));
    WVPASS(g.get("key") == WvString(2));
    WVPASS(g.get("key") == WvString(3));
    WVPASS(g.get("key") == WvString(4));

    g.setgetcallback("key", UniCallbackGenGetCallback());
    WVPASS(!g.get("key"));
}

WVTEST_MAIN("set")
{
    AutoIncrementer ai;
    UniCallbackGen g(1);

    WVPASS(!g.get("key"));

    g.update_after_set = false;
    g.update_before_get = true;
    
    g.setgetcallback("key",
            UniCallbackGenGetCallback(&ai, &AutoIncrementer::get));
    g.setsetcallback("key",
            UniCallbackGenSetCallback(&ai, &AutoIncrementer::set));

    WVPASS(g.get("key") == WvString(0));
    WVPASS(g.get("key") == WvString(1));
    WVPASS(g.get("key") == WvString(2));
    WVPASS(g.get("key") == WvString(3));
    WVPASS(g.get("key") == WvString(4));

    g.set("key", 0);

    WVPASS(g.get("key") == WvString(0));
    WVPASS(g.get("key") == WvString(1));
    WVPASS(g.get("key") == WvString(2));
    WVPASS(g.get("key") == WvString(3));
    WVPASS(g.get("key") == WvString(4));

    g.setgetcallback("key", UniCallbackGenGetCallback());
    WVPASS(!g.get("key"));
}

