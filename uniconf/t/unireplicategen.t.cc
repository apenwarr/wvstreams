#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "unireplicategen.h"

WVTEST_MAIN("basic")
{
    UniConfRoot cfg("replicate:{temp: temp:}");
    WVFAIL(cfg.haschildren());
    WVPASS(cfg["/key"].getme().isnull());
    
    cfg["/key"].setme("value");
    WVPASS(cfg.haschildren());
    WVPASS(cfg["/key"].getme() == "value");
    
    cfg["/key"].setme(WvString::null);
    WVFAIL(cfg.haschildren());
    WVPASS(cfg["/key"].getme().isnull());
}

WVTEST_MAIN("propigation")
{
    UniTempGen tmps[2];
    tmps[0].set("/key0", "value0");
    tmps[0].set("/key", "value0");
    tmps[1].set("/key1", "value1");
    tmps[1].set("/key", "value1");
    
    UniReplicateGen rep;
    
    rep.append(&tmps[1], false);
    WVPASS(!rep.exists("/key0"));
    WVPASS(rep.get("/key1") == "value1");
    WVPASS(rep.get("/key") == "value1");
    WVPASS(!tmps[0].exists("/key1"));
    WVPASS(tmps[0].get("/key") == "value0");
    WVPASS(!tmps[1].exists("/key0"));
    WVPASS(tmps[1].get("/key") == "value1");
   
    rep.prepend(&tmps[0], false);
    WVPASS(rep.get("/key0") == "value0");
    WVPASS(rep.get("/key1") == "value1");
    WVPASS(rep.get("/key") == "value0");
    WVPASS(tmps[0].get("/key1") == "value1");
    WVPASS(tmps[0].get("/key") == "value0");
    WVPASS(tmps[1].get("/key0") == "value0");
    WVPASS(tmps[1].get("/key") == "value0");
    
    rep.set("key", "value");
    WVPASS(rep.get("key") == "value");
    WVPASS(tmps[0].get("key") == "value");
    WVPASS(tmps[1].get("key") == "value");
    
    tmps[0].set("key", "value0");
    WVPASS(rep.get("key") == "value0");
    WVPASS(tmps[0].get("key") == "value0");
    WVPASS(tmps[1].get("key") == "value0");
    
    tmps[1].set("key", "value1");
    WVPASS(rep.get("key") == "value1");
    WVPASS(tmps[0].get("key") == "value1");
    WVPASS(tmps[1].get("key") == "value1");
}
