#include "wvtest.h"
#include "wvtcp.h"
#include "wvistreamlist.h"

static void lcallback(WvStream&, void *userdata)
{
    WvTCPListener *l = (WvTCPListener *)userdata;
    WvTCPConn *conn = l->accept();
    WvIStreamList::globallist.append(conn, true);
}

WVTEST_MAIN("tcp establish connection")
{
    int port = 0;

    WvString laddrstr("0.0.0.0:%s", port);
    WvIPPortAddr laddr(laddrstr);
    WvTCPListener l(laddr);
    l.setcallback(lcallback, &l);
    WvIStreamList::globallist.append(&l, false);

    printf("Starting listener on %s\n", laddrstr.cstr());

    WVPASS(l.isok());

    if (!l.isok())
	wvcon->print("Error was: %s\n", l.errstr());

    // get port we actually bound to
    port = l.src()->port;

    printf("Using port %d\n", port);

    WvString caddrstr("127.0.0.1:%s", port);
    WvIPPortAddr caddr(caddrstr);
    WvTCPConn c(caddr);
    WvIStreamList::globallist.append(&c, false);

    WVPASS(c.isok());

    if (!c.isok())
	wvcon->print("Error was: %s\n", c.errstr());

    WVPASS(c.select(0));
}

