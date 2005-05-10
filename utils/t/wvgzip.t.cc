#include "wvgzip.h"
#include "wvtest.h"

const int PATTERN_LENGTH = 2;
const int NUM_REPEATS = 500;
const size_t STRING_LENGTH = PATTERN_LENGTH * NUM_REPEATS;

WVTEST_MAIN("wvgzip trivial encode + decode x1")
{
    WvString str;
    for (int i=0; i<NUM_REPEATS; i++)
        str.append("10");

    WvDynBuf inbuf;
    inbuf.putstr(str);
    WvDynBuf zippedbuf;
    WvDynBuf unzippedbuf;
    printf("inbuf: %i zippedbuf: %i\n", inbuf.used(), zippedbuf.used());

    WvGzipEncoder zipper(WvGzipEncoder::Deflate);
    zipper.encode(inbuf, zippedbuf, true, true);
    WVPASS(zippedbuf.used() < STRING_LENGTH);
    WVPASS(zippedbuf.used() > 0);

    WvGzipEncoder unzipper(WvGzipEncoder::Inflate);
    unzipper.encode(zippedbuf, unzippedbuf, true, true);
    printf("inbuf: %i unzippedbuf: %i\n", inbuf.used(), unzippedbuf.used());
    WVPASS(unzippedbuf.used() == STRING_LENGTH);
    WvString unzippedstr = unzippedbuf.getstr(); 
    WVPASS(unzippedstr == str);
}


WVTEST_MAIN("wvgzip trivial encode + decode x2")
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
    comp.zap();
    uncomp.zap();
    srand(time(NULL));
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 32768; j++)
            buf[j] = rand() % 256;

        uncomp.put(buf, 32768);
    }
    WVPASSEQ(uncomp.used(), 65536);
    gzencdef.reset();
    gzencdef.encode(uncomp, comp, true);

    // Make sure it read everything.
    WVPASSEQ(uncomp.used(), 0);

    gzencinf.reset();
    gzencinf.out_limit = 20480-1;

    unsigned int i = 0;
    do
    {
        i++;
        gzencinf.encode(comp, uncomp, true);
        WVPASS(uncomp.used() <= i*(20480-1));
        WVPASS(gzencinf.isok());
    } while (comp.used());

    WVPASSEQ(uncomp.used(), 65536);

    // Further encoding shouldn't do anything.
    gzencinf.encode(comp, uncomp, true);
    WVPASSEQ(uncomp.used(), 65536);
    WVPASS(gzencinf.isok());
}
