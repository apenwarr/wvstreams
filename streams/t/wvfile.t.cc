/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvfile.h"

WVTEST_MAIN("basics")
{
    WvFile f("/dev/zero", O_RDONLY);
    WVPASS(f.isok());
    if (!f.isok())
	printf("file error code: '%s'\n", f.errstr().cstr());
    WVPASS(f.isreadable());
    WVFAIL(f.iswritable());
}
