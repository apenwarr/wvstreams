#include "wvtest.h"
#include "wvstream.h"
#include "wvstreamclone.h"

WVTEST_MAIN("close() non-loopiness")
{
    WvStream *s = new WvStream();
    WvStreamClone c(s);
    c.close();
}
