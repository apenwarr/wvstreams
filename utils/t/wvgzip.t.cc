#include "wvgzip.h"
#include "wvtest.h"

// currently disabled: see bug 3856
#if 0
const int PATTERN_LENGTH = 2;
const int NUM_REPEATS = 500;
const size_t STRING_LENGTH = PATTERN_LENGTH * NUM_REPEATS;

WVTEST_MAIN("wvgzip trivial encode + decode")
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
#endif
