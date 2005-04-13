#include "wvgzip.h"
#include "wvtest.h"

WVTEST_MAIN("output limiting")
{
    size_t bufsize;
    char buf[40000];
    memset(buf, 0, 32768);

    WvDynBuf uncomp, comp;
    uncomp.put(buf, 32768);

    WvGzipEncoder gzencdef(WvGzipEncoder::Deflate);
    gzencdef.encode(uncomp, comp, true);

    // Make sure it read everything.
    WVPASSEQ(uncomp.used(), 0);

    // Store compressed data for later tests.
    bufsize = comp.used();
    memcpy(buf, comp.get(bufsize), bufsize);

    comp.put(buf, bufsize);

    // Test without output limiting.  Should do everything in one step.
    WvGzipEncoder gzencinf(WvGzipEncoder::Inflate);
    gzencinf.encode(comp, uncomp, true);

    WVPASSEQ(uncomp.used(), 32768);
    WVPASSEQ(comp.used(), 0);

    uncomp.zap();
    comp.put(buf, bufsize);
    gzencinf.reset();

    // Test with an out_limit by which the buffer is evenly divisible.
    gzencinf.out_limit = 1024;

    for (int i = 1; i <= 32; i++)
    {
        gzencinf.encode(comp, uncomp, true);
        WVPASSEQ(uncomp.used(), i*1024);
        WVPASS(gzencinf.isok());
    }

    uncomp.zap();
    comp.put(buf, bufsize);
    gzencinf.reset();

    // Test with an out_limit by which the buffer isn't evenly divisible
    // (i.e. with a remainder).
    gzencinf.out_limit = 10240-1;

    for (int i = 1; i <= 3; i++)
    {
        gzencinf.encode(comp, uncomp, true);
        WVPASSEQ(uncomp.used(), i*(10240-1));
        WVPASS(gzencinf.isok());
    }

    // The remainder.
    gzencinf.encode(comp, uncomp, true);
    WVPASSEQ(uncomp.used(), 32768);
    WVPASS(gzencinf.isok());

    // Further encoding shouldn't do anything.
    gzencinf.encode(comp, uncomp, true);
    WVPASSEQ(uncomp.used(), 32768);
    WVPASS(gzencinf.isok());

    // Try with a random-content buffer.
    srand(time(NULL));
    for (int i = 0; i < 32768; i++)
        buf[i] = rand() % 256;

    comp.zap();
    uncomp.zap();
    uncomp.put(buf, 32768);

    gzencdef.reset();
    gzencdef.encode(uncomp, comp, true);

    // Make sure it read everything.
    WVPASSEQ(uncomp.used(), 0);

    gzencinf.reset();
    gzencinf.out_limit = 10240-1;

    unsigned int i = 0;
    do
    {
        i++;
        gzencinf.encode(comp, uncomp, true);
        WVPASS(uncomp.used() <= i*(10240-1));
        WVPASS(gzencinf.isok());
    } while (comp.used());

    WVPASSEQ(uncomp.used(), 32768);

    // Further encoding shouldn't do anything.
    gzencinf.encode(comp, uncomp, true);
    WVPASSEQ(uncomp.used(), 32768);
    WVPASS(gzencinf.isok());
}

