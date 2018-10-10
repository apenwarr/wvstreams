#include "uniautogen.h"
#include "uniconfroot.h"
#include "wvlinkerhack.h"
#include "wvtest.h"

WV_LINK_TO(UniDefGen);
WV_LINK_TO(UniIniGen);


WVTEST_MAIN("basics")
{
    uniautogen_moniker = "default:ini:tmp.ini";
    
    {
	UniConfRoot r("ini:tmp.ini");
	r.xset("/a/b/c", "ini:tmp.ini");
	r.xset("/a/*", "ini:tmp2.ini");
	r.xset("/a", "ini:tmp3.ini");
	r.commit();
    }
    
    {
	UniConfRoot r("ini:tmp2.ini");
	r.xset("/2/2/2", "tmp2.ini");
	r.xset("/1/1/1", "also tmp2.ini");
	r.commit();
    }

    {
	UniConfRoot r("ini:tmp3.ini");
	r.xset("/3/3/3", "tmp3.ini");
	r.xset("/1/1/1", "also tmp3.ini");
	r.commit();
    }
    
    WVPASSEQ(UniConfRoot("auto:a/b/c").xget("a/b/c"), "ini:tmp.ini");
    WVPASSEQ(UniConfRoot("auto:a/b/c/a/b").xget("c"), "ini:tmp.ini");
    WVPASSEQ(UniConfRoot("auto:a/x").xget("2/2/2"), "tmp2.ini");
    WVPASSEQ(UniConfRoot("auto:a/*").xget("2/2/2"), "tmp2.ini");
    WVPASSEQ(UniConfRoot("auto:a/x/2/2").xget("2"), "tmp2.ini");
    WVPASSEQ(UniConfRoot("auto:a").xget("3/3/3"), "tmp3.ini");
}
