#include "wvtest.h"
#include "wvstream.h"
#include "wvstreamclone.h"
#include "wvloopback.h"

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


WVTEST_MAIN("streamclone eof1")
{
    WvStreamClone s(new WvLoopback);
    s.nowrite(); // done sending
    s.blocking_getline(1000);
    WVFAIL(s.isok()); // should be eof now
}


WVTEST_MAIN("streamclone eof2")
{
    WvStreamClone s(new WvLoopback);
    s.write("Hello\n");
    s.write("nonewline");
    s.nowrite();
    WVPASS(s.isok());
    WVPASSEQ(s.blocking_getline(100), "Hello");
    WVPASS(s.isok());
    WVPASSEQ(s.blocking_getline(100), "nonewline");
    WVFAIL(s.isok());
}
