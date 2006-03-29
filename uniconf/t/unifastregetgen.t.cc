#include "unifastregetgen.h"
#include "unislowgen.h"
#include "unitempgen.h"
#include "uniconfroot.h"
#include "wvtest.h"
#include "uniconfgen-sanitytest.h"

    
WVTEST_MAIN("UniFastRegetGen Sanity Test")
{
    UniFastRegetGen *gen = new UniFastRegetGen(new UniTempGen);
    UniConfGenSanityTester::sanity_test(gen, "fast-reget:temp:");
    WVRELEASE(gen);
}

WVTEST_MAIN("fast-reget")
{
    UniTempGen *t = new UniTempGen;
    UniSlowGen *slow = new UniSlowGen(t);
    UniConfRoot uni(new UniFastRegetGen(slow), true);
    
    slow->reset_slow();
    
    // initial get needs to retrieve entire tree
    t->set("x/y/z", 5);
    WVPASSEQ(uni.xgetint("x/y/z"), 5);
    WVPASSEQ(slow->how_slow(), 3); slow->reset_slow();
    
    // regets are free
    WVPASSEQ(uni.xgetint("x/y/z"), 5);
    WVPASSEQ(uni.xgetint("x/y/z"), 5);
    WVPASSEQ(slow->how_slow(), 0); slow->reset_slow();
    
    // notifications are processed
    uni.xset("x/y/z", 7);
    WVPASSEQ(uni.xgetint("x/y/z"), 7);
    WVPASSEQ(slow->how_slow(), 0); slow->reset_slow();

    // updates directly inside inner generator are free
    t->set("x/y/z", 9);
    t->set("x", 10);
    WVPASSEQ(uni.xgetint("x/y/z"), 9);
    WVPASSEQ(uni.xgetint("x"), 10);
    WVPASSEQ(slow->how_slow(), 0); slow->reset_slow();
    
    // known null parents should make child lookups free
    WvString nil;
    WVPASSEQ(uni.xget("a/b/c"), nil);
    slow->reset_slow();
    WVPASSEQ(uni.xget("a/b/d"), nil);
    WVPASSEQ(slow->how_slow(), 0); slow->reset_slow();
    
    // but a parent then becoming non-null should not automatically make
    // an already-looked-up child non-free
    t->set("a/b", 99);
    WVPASSEQ(uni.xget("a/b/d"), nil);
    WVPASSEQ(slow->how_slow(), 0); slow->reset_slow();
    
    // checking children of non-nil nodes takes time and is not cached
    WVPASSEQ(uni["a/b"].haschildren(), false);
    WVPASSEQ(uni["a/b"].haschildren(), false);
    WVPASSEQ(slow->how_slow(), 2); slow->reset_slow();
    
    // but checking children of nil nodes is trivial, so it's fast
    WVPASSEQ(uni["a/b/d"].haschildren(), false);
    WVPASSEQ(slow->how_slow(), 0); slow->reset_slow();
}
