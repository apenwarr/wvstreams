#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"

WVTEST_MAIN("even more basic")
{
    UniTempGen g;
    WVFAIL(g.haschildren("/"));
    
    g.set("/", "blah");
    WVFAIL(g.haschildren("/"));
    
    g.set("/x", "pah");
    WVPASS(g.haschildren("/"));
    WVFAIL(g.haschildren("/x"));
    
    g.set("/", WvString());
    WVFAIL(g.haschildren("/"));
}


WVTEST_MAIN("basics")
{
    UniConfRoot cfg("temp:");
    WVFAIL(cfg.haschildren());
    
    cfg.set("blah");
    WVFAIL(cfg.haschildren());
    
    cfg["x"].set("pah");
    WVPASS(cfg.haschildren());
    WVFAIL(cfg["x"].haschildren());
    
    cfg.remove();
    WVFAIL(cfg.haschildren());
}

// FIXME: could test lots more stuff here...
