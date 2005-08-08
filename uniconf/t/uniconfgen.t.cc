#include "wvtest.h"
#include "uniconfgen.h"
#include "wvmoniker.h"

static void cb(const UniConfKey &, WvStringParm)
{
}


WVTEST_MAIN("null generator, setcallback, delete")
{
    IUniConfGen *gen = wvcreate<IUniConfGen>("null:");
    WVPASS(gen);
    if (gen)
    {
	gen->add_callback(gen, cb);
	gen->del_callback(gen);
    }
    WVRELEASE(gen);
}
