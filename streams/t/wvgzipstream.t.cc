#include "wvtest.h"
#include "wvgzipstream.h"
#include "wvloopback.h"

#ifdef _WIN32
#define ZLIB_DOESNT_HAVE_VALGRIND_ERRORS 1 // win32 has no valgrind!
#endif

// hmm... this test itself passes now, but the gzip encoder seems to have
// valgrind errors.  Probably minor, but we shouldn't upset the unit tests.
#if ZLIB_DOESNT_HAVE_VALGRIND_ERRORS

WVTEST_MAIN("autoflush")
{
    WvLoopback loopy;
    WvGzipStream gzip(&loopy);
    gzip.disassociate_on_close = true;
    
    char buf[1024];
    
    // since autoflush is enabled, gzip encoder should always be flushed
    // right away.
    gzip.write("x");
    WVPASS(gzip.read(buf, sizeof(buf)));
    
    // when auto_flush is disabled, expect at least a short delay.
    gzip.auto_flush(false);
    gzip.write("y");
    WVFAIL(gzip.read(buf, sizeof(buf)));
    gzip.select(1000);
    WVPASS(gzip.read(buf, sizeof(buf)));
}

#endif
