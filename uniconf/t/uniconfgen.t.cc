#include "wvtest.h"
#include "uniconfgen.h"
#include "wvmoniker.h"

static void cb(const UniConfKey &, WvStringParm, void *)
{
}


WVTEST_MAIN("null generator, setcallback, delete")
{
    IUniConfGen *gen = wvcreate<IUniConfGen>("null:");
    WVPASS(gen);
    if (gen)
	gen->setcallback(cb, NULL);
    delete gen;
}
