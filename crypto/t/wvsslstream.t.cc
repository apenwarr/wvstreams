#include "wvtest.h"
#include "wvsslstream.h"
#include "wvloopback2.h"
#include "wvx509.h"
#include "wvistreamlist.h"
#include <signal.h>

static void run(WvStream &list, WvStream &s1, WvStream &s2)
{
    for (int i = 0; i < 100; i++)
    {
	list.runonce(10);
	if (s1.isreadable() || s2.isreadable())
	    break;
    }
}

WVTEST_MAIN("crypto basics")
{
    signal(SIGPIPE, SIG_IGN);
    
    WvStream nil;
    IWvStream *_s1, *_s2;
    wv_loopback2(_s1, _s2);
    
    WvX509Mgr x509("cn=random_stupid_dn", 1024);
    WvSSLStream s1(_s1, &x509, 0, true);
    WvSSLStream s2(_s2);
    
    WvIStreamList list;
    list.append(&s1, false);
    list.append(&s2, false);
    
    // test some basic data transfer
    WVPASS(s1.isok());
    WVPASS(s2.isok());
    s1.print("s1 output\ns1 line2\n");
    s2.print("s2 output\ns2 line2\n");
    run(list, s1, s2);
    WVPASSEQ(s1.blocking_getline(1000), "s2 output");
    WVPASSEQ(s2.blocking_getline(1000), "s1 output");
    WVPASSEQ(s2.getline(), "s1 line2");
    WVPASSEQ(s1.blocking_getline(1000), "s2 line2");
    
    // test special close() behaviour.  At one point, WvSSLStream::uread()
    // would return nonzero, but close the stream anyway *before* returning.
    // This causes very confusing behaviour, because the calling stream
    // receives the SSL stream's closecallback() before getting the final
    // data from uread().  Thus, uread() should only call close() if it's
    // planning to return 0.
    s2.nowrite();
    s1.print("foostring");
    run(list, s1, s2);
    s1.close();
    run(list, s2, nil);
    WvDynBuf buf;
    WVPASSEQ(s2.read(buf, 1024), 9);
    WVPASSEQ(buf.getstr(), "foostring");
    WVPASS(s2.isok());
    run(list, s2, nil);
    WVPASSEQ(s2.read(buf, 1024), 0);
    WVFAIL(s2.isok());
}
