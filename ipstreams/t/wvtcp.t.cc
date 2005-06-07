#include "wvtest.h"
#include "wvtcp.h"
#include "wvistreamlist.h"

#define BASEPORT	11223

static void lcallback(WvStream&, void *userdata)
{
    WvTCPListener *l = (WvTCPListener *)userdata;
    WvTCPConn *conn = l->accept();
    WvIStreamList::globallist.append(conn, true);
}

WVTEST_MAIN("tcp establish connection")
{
    int port = BASEPORT + getpid();

    WvString laddrstr("0.0.0.0:%s", port);
    WvIPPortAddr laddr(laddrstr);
    WvTCPListener l(laddr);
    l.setcallback(lcallback, &l);
    WvIStreamList::globallist.append(&l, false);

    printf("Starting listener on %s\n", laddrstr.cstr());

    WVPASS(l.isok());

    WvString caddrstr("127.0.0.1:%s", port);
    WvIPPortAddr caddr(caddrstr);
    WvTCPConn c(caddr);
    WvIStreamList::globallist.append(&c, false);

    WVPASS(c.isok());

    WVPASS(c.select(0));
}

