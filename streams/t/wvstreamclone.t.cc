#include "wvtest.h"
#include "wvstream.h"
#include "wvstreamclone.h"

WVTEST_MAIN("close() non-loopiness")
{
    WvStream *s = new WvStream();
    WvStreamClone c(s);
    c.close();
}


// noread/nowrite behaviour
WVTEST_MAIN("noread/nowrite")
{
    WvStream s1;
    WvStreamClone s(&s1);
    s.disassociate_on_close = true;
    char buf[1024];

    s.nowrite();
    WVPASS(s.isok());
    WVFAIL(s.write(buf, 1024) != 0);
    s.noread();
    WVPASS(!s.isok());
}
