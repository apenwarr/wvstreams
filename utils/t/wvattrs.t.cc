#include "wvattrs.h"
#include "wvtest.h"

WVTEST_MAIN("getattr/setattr")
{
    WvAttrs *one = new WvAttrs;
    WvAttrs &s = *one;

    WVPASSEQ(s.getattr("not set!"), "");
    s.setattr("set1", "val1");
    WVPASSEQ(s.getattr("set1"), "val1");
    s.setattr("set2", "val2");
    WVPASSEQ(s.getattr("set2"), "val2");
    s.setattr("set3", "value3");
    s.setattr("set2", "value2");
    WVPASSEQ(s.getattr("set2"), "value2");
    WVPASSEQ(s.getattr("set3"), "value3");
    s.setattr("set1", "");
    WVPASSEQ(s.getattr("set1"), "");
    s.setattr("", "this should not be set");
    s.setattr("seter4", "some value");
    s.setattr("seter5", "some other value");
    s.setattr("setter6", "OK, last one");
    WVPASSEQ(s.getattr("seter5"), "some other value");

    //test copy constructor
    WvAttrs copy(s);
    delete one;

    WVPASSEQ(copy.getattr("seter4"), "some value");
}
