#include "wvtest.h"
#include "wvstream.h"
#include "wvstreamclone.h"

WVTEST_MAIN("bug 3184")
{
    WvStream *s = new WvStream();
    WvStreamClone c(s);
    c.close();
}
