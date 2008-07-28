#include "wvattrs.h"
#include "wvtest.h"

WVTEST_MAIN("get/set")
{
    WvAttrs *one = new WvAttrs;
    WvAttrs &s = *one;

    WVPASSEQ(s.get("not set!"), "");
    s.set("set1", "val1");
    WVPASSEQ(s.get("set1"), "val1");
    s.set("set2", "val2");
    WVPASSEQ(s.get("set2"), "val2");
    s.set("set3", "value3");
    s.set("set2", "value2");
    WVPASSEQ(s.get("set2"), "value2");
    WVPASSEQ(s.get("set3"), "value3");
    s.set("set1", "");
    WVPASSEQ(s.get("set1"), "");
    s.set("", "this should not be set");
    s.set("seter4", "some value");
    s.set("seter5", "some other value");
    s.set("setter6", "OK, last one");
    WVPASSEQ(s.get("seter5"), "some other value");

    //test copy constructor
    WvAttrs copy(s);
    delete one;

    WVPASSEQ(copy.get("seter4"), "some value");
}
