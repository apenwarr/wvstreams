#include "wvtest.h"
#include "uniconfdaemon.h"
#include "wvstringlist.h"
#include "wvtclstring.h"
#include "uniconfroot.h"
#include "wvtcp.h"
#include "wvistreamlist.h"
#include <signal.h>


static void spin(WvIStreamList &l)
{
    int max;
    for (max = 0; max < 100 && l.select(10); max++)
    {
	wvcon->print(".");
	l.callback();
    }
    wvcon->print("\n");
    WVPASS(max < 100);
}


static void appendbuf(WvStream &s, void *_buf)
{
    printf("append!\n");
    WvDynBuf *buf = (WvDynBuf *)_buf;
    s.read(*buf, 10240);
}


static void linecmp(WvIStreamList &sl, WvBuf &buf,
		    const char *w1, const char *w2 = NULL,
		    const char *w3 = NULL)
{
    printf("%s", WvString("Awaiting '%s' '%s' '%s'\n", w1, w2, w3).cstr());
    spin(sl);
    
    WvString line = wvtcl_getword(buf, "\r\n");
    WVPASS(line);
    if (!line) return;
    
    WvStringList l;
    wvtcl_decode(l, line);
    
    size_t nargs = w3 ? 3 : (w2 ? 2 : 1);
    WVPASS(l.count() < 4);
    
    WVPASSEQ(l.popstr(), w1);
    if (nargs >= 2) WVPASSEQ(l.popstr(), w2);
    if (nargs >= 3) WVPASSEQ(l.popstr(), w3);
}


WVTEST_MAIN("daemon surprise close")
{
    WvIStreamList l;
    WVPASSEQ(WvIStreamList::globallist.count(), 0);
    spin(WvIStreamList::globallist);
    
    signal(SIGPIPE, SIG_IGN);
    WvIPPortAddr addr("0.0.0.0:4113");
    
    UniConfRoot cfg("temp:");
    UniConfDaemon daemon(cfg, false, NULL);
    
    spin(l);
    
    WVPASS(daemon.isok());
    daemon.setuptcpsocket(addr);
    WVPASS(daemon.isok());
    
    spin(l);
    
    WvDynBuf buf;
    WvTCPConn tcp(addr);
    tcp.setcallback(appendbuf, &buf);
    WVPASS(tcp.isok());
    
    l.append(&daemon, false);
    l.append(&tcp, false);
    
    linecmp(l, buf, "HELLO");
    WVPASS(tcp.isok());

    tcp.write("SET /x/y z\n");
    tcp.close();
    spin(l);
    //linecmp(l, buf, "NOTICE", "", "");
    //linecmp(l, buf, "NOTICE", "x", "");
    //linecmp(l, buf, "NOTICE", "x/y", "z");
}
