#include "wvtest.h"
#include "unicallbackgen.h"
#include "uniconfgen-sanitytest.h"

class AutoIncrementer
{
    int value;
    
public:
    
    AutoIncrementer() : value(0) {}
    
    WvString get(const UniConfKey &) { return value++; }
    void set(const UniConfKey &, WvStringParm _value) { value = _value.num(); }
};

WVTEST_MAIN("UniCallbackGen Sanity Test")
{
    UniCallbackGen *gen = new UniCallbackGen(1);
    // The callback generator doesn't have a moniker
    UniConfGenSanityTester::sanity_test(gen, WvString::null);
    WVRELEASE(gen);
}

WVTEST_MAIN("non-dynamic")
{
    AutoIncrementer ai;
    UniCallbackGen g(1);

    WVPASS(!g.get("key"));
    
    g.setgetcallback("key", wv::bind(&AutoIncrementer::get, &ai, wv::_1));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(0));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(1));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(2));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(3));
    WVPASS((g.set("key", "foo"), g.get("key")) == WvString(4));

    g.setgetcallback("key", UniCallbackGenGetCallback());
    WVPASS((g.set("key", "foo"), g.get("key") == WvString("foo")));
}

WVTEST_MAIN("dynamic")
{
    AutoIncrementer ai;
    UniCallbackGen g(1);

    WVPASS(!g.get("key"));

    g.update_after_set = false;
    g.update_before_get = true;
    
    g.setgetcallback("key", wv::bind(&AutoIncrementer::get, &ai, wv::_1));

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
    
    g.setgetcallback("key", wv::bind(&AutoIncrementer::get, &ai, wv::_1));
    g.setsetcallback("key", wv::bind(&AutoIncrementer::set, &ai, wv::_1,
				     wv::_2));

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

