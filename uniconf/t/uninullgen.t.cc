#include "wvtest.h"
#include "uniconfroot.h"


WVTEST_MAIN("nullgen basics")
{
    UniConfRoot cfg("null:");
    WVFAIL(cfg.haschildren());
    
    cfg.set("blah");
    WVFAIL(cfg.haschildren());
    
    cfg["x"].set("pah");
    WVFAIL(cfg.haschildren());
    WVFAIL(cfg["x"].exists());
    WVFAIL(cfg["x"].haschildren());
    
    cfg.remove();
    WVFAIL(cfg.haschildren());
}
