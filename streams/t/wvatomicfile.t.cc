/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvatomicfile.h"

WVTEST_MAIN("atomic file test")
{
    WvAtomicFile f("/dev/zero", O_RDONLY);
    WVPASS(f.isok() && !f.isatomic());
    if (!f.isok())
	printf("file error code: '%s'\n", f.errstr().cstr());

    WVPASS(f.isreadable());
    WVFAIL(f.iswritable());

    f.close();

    WvAtomicFile g("/tmp/testfile", O_RDWR | O_CREAT);
    WVPASS(g.isreadable());
    WVPASS(g.iswritable());
    WVPASS(g.isok() && g.isatomic());

    g.close();
    ::unlink("testfile");
}
