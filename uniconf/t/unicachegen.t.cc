#include "unitempgen.h"
#include "unicachegen.h"
#include "uniconfroot.h"
#include "uniwatch.h"
#include "wvtest.h"

int temp_cbs_received = 0;

class WatchReceiver
{
public:
    WatchReceiver(WvStringParm _expected_key,
                  WvStringParm _expected_value) : 
        cbs(0),
        expected_key(_expected_key),
        expected_value(_expected_value) {}
    void callback(const UniConf keyconf, const UniConfKey key) 
        { 
            printf("key: %s expected key: %s\n", key.cstr(), expected_key.cstr());
            WVPASS(key == expected_key);
            WVPASS(keyconf[key].getme() == expected_value);
            cbs++;
        }
    int cbs;
private:
    WvString expected_key;
    WvString expected_value;
};

WVTEST_MAIN("cache basics")
{
    UniTempGen *t = new UniTempGen;
    UniConfRoot cacheroot;
    UniCacheGen *c = new UniCacheGen(t);
    cacheroot.mountgen(c, true);
    
    // 1 level of hierarchy
    t->set("foo", "bar");
    WVPASS(cacheroot["foo"].getme() == "bar");
    WVFAIL(cacheroot["foo"].haschildren());

    cacheroot["bar"].setme("foo");
    WVPASS(t->get("bar") == "foo");
    WVFAIL(t->haschildren("bar"));

    // 3 levels of hierarchy
    t->set("foo/baz/beep", "bar");
    WVPASS(cacheroot["foo/baz/beep"].getme() == "bar");
    WVFAIL(cacheroot["foo/baz/beep"].haschildren());

    cacheroot["foo/baz/boop"].setme("foo");
    WVPASS(t->get("foo/baz/boop") == "foo");
    WVFAIL(t->haschildren("foo/baz/boop"));
    
    // (recursive) notifications of the cache
    WatchReceiver cache_received("beep", "foo");

    UniWatch cache_watch(cacheroot["/foo/baz"], 
                         UniConfCallback(&cache_received,
                                         &WatchReceiver::callback));
    
    t->set("foo/baz/beep", "foo");
    WVPASS(cache_received.cbs == 1);

    // make sure that we are _not_ notified if the
    // key is not a subkey of the one we are watching
    t->set("foo", "foo");
    WVPASS(cache_received.cbs == 1);
}
