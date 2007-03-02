/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvfile.h"

WVTEST_MAIN("basics")
{
    WvFile f("wvfile.t.tmp", O_WRONLY | O_CREAT);
    WVPASS(f.isok());
    if (!f.isok())
	printf("file error code: '%s'\n", f.errstr().cstr());
    WVFAIL(f.isreadable());
    WVPASS(f.iswritable());
}
