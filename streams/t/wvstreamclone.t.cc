#include "wvtest.h"
#include "wvstream.h"
#include "wvstreamclone.h"
#include "wvloopback.h"
#include "wvsocketpair.h"

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
    WVPASSEQ(s.blocking_getline(1000), "Hello");
    WVPASS(s.isok());
    WVPASSEQ(s.blocking_getline(1000), "nonewline");
    WVFAIL(s.isok());
}


WVTEST_MAIN("streamclone eof3")
{
    WvLoopback *l = new WvLoopback;
    WvStreamClone s(l);
    s.write("barfdata", 8);
    
    // read the full 8 bytes into the stream's inbuf, but there's no newline,
    // so return NULL
    WVFAIL(s.blocking_getline(200));
    s.queuemin(0);
    
    char buf[8];
    WVPASSEQ(s.read(buf, 7), 7);
    WVPASS(s.isok()); // one block received last time; no EOF yet
    WVPASS(s.isreadable());
    
    // all the data has been sent, but one byte remains in inbuf
    l->close();
    
    WVPASS(s.isok()); // no EOF received by wrapper yet!
    WVPASS(s.isreadable());
    WVPASSEQ(s.read(buf, 8), 1);
    
//    WVPASS(s.isok()); // STILL no EOF received
//    WVPASS(s.isreadable());
    
    // get EOF
    WVPASSEQ(s.read(buf, 8), 0);
    WVFAIL(s.isok()); // got EOF, so should now definitely be shut down
}


WVTEST_MAIN("cloned inbuf after read error")
{
    int socks[2];
    WVPASS(!wvsocketpair(SOCK_STREAM, socks));
    WvStreamClone s1(new WvFdStream(socks[0])), s2(new WvFdStream(socks[1]));
    s1.print("1\n2\n3\n4\n");
    WVPASSEQ(s2.blocking_getline(1000), "1");
    s1.close();
    WvStreamClone ss2(&s2);
    ss2.disassociate_on_close = true;
    
    WVPASSEQ(ss2.blocking_getline(1000), "2");
    s2.close(); // underlying stream goes away
    WVPASSEQ(ss2.blocking_getline(1000), "3");
    WVPASS(ss2.isok()); // inbuf still has data!  Don't detect this yet.
    WVPASSEQ(ss2.blocking_getline(1000), "4");
    WVFAIL(ss2.blocking_getline(1000));
    WVFAIL(ss2.isok());
}


WVTEST_MAIN("WvStreamClone setclone behaviour")
{
    WvStream s1;
    WvStream s2;
    WvStreamClone s(&s1);

    s.disassociate_on_close = true;

    WVPASS(s.isok());
    s.noread();
    WVPASS(s.isok());
    s.nowrite();
    WVPASS(!s.isok());
    WVPASS(!s1.isok());
    WVPASS(s2.isok());
    s.setclone(&s2);
    WVPASS(s.isok());
    WVPASS(s2.isok());
    s.setclone(NULL);
    WVPASS(!s.isok());
}

