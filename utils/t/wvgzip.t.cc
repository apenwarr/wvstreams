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
    printf("inbuf: %i zippedbuf: %i\n", (int)inbuf.used(), (int)zippedbuf.used());

    WvGzipEncoder zipper(WvGzipEncoder::Deflate);
    zipper.encode(inbuf, zippedbuf, true, true);
    WVPASS(zippedbuf.used() < STRING_LENGTH);
    WVPASS(zippedbuf.used() > 0);

    WvGzipEncoder unzipper(WvGzipEncoder::Inflate);
    unzipper.encode(zippedbuf, unzippedbuf, true, true);
    printf("inbuf: %i unzippedbuf: %i\n", (int)inbuf.used(), (int)unzippedbuf.used());
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
    const int EVEN_OUT_LIMIT = 1024;
    gzencinf.out_limit = EVEN_OUT_LIMIT;

    for (int i = 1; i <= 32; i++)
    {
        gzencinf.encode(comp, uncomp, true);
        WVPASSEQ(uncomp.used(), i*EVEN_OUT_LIMIT);
        WVPASS(gzencinf.isok());
    }

    uncomp.zap();
    comp.put(buf, bufsize);
    gzencinf.reset();

    // Test with an out_limit by which the buffer isn't evenly divisible
    // (i.e. with a remainder). Also make it bigger than the buffer,
    // to be sure that we keep on decompressing (BUGZID:20720).
    const int UNEVEN_OUT_LIMIT = (10240+16);
    gzencinf.out_limit = UNEVEN_OUT_LIMIT;

    for (int i = 1; i <= 3; i++)
    {
        gzencinf.encode(comp, uncomp, true);
        WVPASSEQ(uncomp.used(), i*(UNEVEN_OUT_LIMIT));
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


WVTEST_MAIN("compression errors")
{
    WvDynBuf comp, uncomp;
    WvGzipEncoder gzipdef(WvGzipEncoder::Deflate),
                  gzipinf(WvGzipEncoder::Inflate);
    gzipdef.full_flush = true;
    gzipinf.ignore_decompression_errors = true;

    char inbuf[101], outbuf[101];
    srand(time(NULL));
    for (int i = 0; i < 100; i++)
        inbuf[i] = (rand() % 10) + 0x30;
    inbuf[100] = '\0';

    // Deflate data in two separate, fully flushed blocks.
    uncomp.put(inbuf, 50);
    gzipdef.encode(uncomp, comp, true);
    uncomp.put(&inbuf[50], 50);
    gzipdef.encode(uncomp, comp, true);
    WVPASS(gzipdef.isok());

    // Corrupting the first two bytes will result in completely broken
    // data, so corrupt byte 3 to test partially recoverable data.
    size_t comp_used = comp.used();
    memcpy(outbuf, comp.get(comp_used), comp_used);
    WVPASSEQ(comp.used(), 0);
    outbuf[2] = '!';

    comp.put(outbuf, comp_used);
    gzipinf.encode(comp, uncomp, true, true);
    WVPASS(gzipinf.isok());

    size_t uncomp_used = uncomp.used();
    memcpy(outbuf, uncomp.get(uncomp_used), uncomp_used);
    outbuf[uncomp_used] = '\0';

    // We should end up with just the second block completely decompressed
    // with no errors.
    WVPASSEQ(uncomp_used, 50);
    WVPASSEQ(memcmp(&inbuf[50], outbuf, uncomp_used), 0);
}


WVTEST_MAIN("severe compression errors")
{
    WvDynBuf comp, uncomp;
    WvGzipEncoder gzipdef(WvGzipEncoder::Deflate),
    gzipinf(WvGzipEncoder::Inflate);
    gzipdef.full_flush = true;
    gzipinf.ignore_decompression_errors = true;
    
    char inbuf[32000], outbuf[32768];
    srand(time(NULL)); 
    for (int i = 0; i < 31999; i++)
        inbuf[i] = (rand() % 10) + 0x30;
    inbuf[31999] = '\0';
    
    uncomp.put(inbuf, 32000);
    gzipdef.encode(uncomp, comp, true);
    gzipdef.finish(comp);
    WVPASS(gzipdef.isok());
    
    size_t comp_used = comp.used();
    wvcon->print("comp_used is %s\n", comp_used);
    comp.get(12);
    comp_used -= 12;
    memcpy(outbuf, comp.get(comp_used), comp_used);
    WVPASSEQ(comp.used(), 0);
    
    char *outbufp = outbuf;
    uncomp.zap(); 
    WVPASS(gzipinf.isok());
    
    size_t decoded = 0;
    do
    {
        size_t to_decode = comp_used - (outbufp - outbuf) < 1024
        ? comp_used - (outbufp - outbuf) : 1024;
        decoded += to_decode;
        comp.put(outbufp, to_decode);
        outbufp += to_decode;
        wvcon->print("Decoding %s bytes.\n", to_decode);
        gzipinf.encode(comp, uncomp, true);
    } while (outbufp < outbuf + comp_used);

    wvcon->print("Decoded %s bytes.\n", decoded);
    gzipinf.finish(uncomp);
    if (!gzipinf.isok())
        wvcon->print("GzipEncoder error: %s\n", gzipinf.geterror());
}
