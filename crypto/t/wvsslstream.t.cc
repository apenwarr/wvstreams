#include "wvtest.h"
#include "wvtcp.h"
#include "wvistreamlist.h"
#include "wvx509.h"
#include "wvrsa.h"
#include "wvsslstream.h"
#include "wvcrypto.h"

#define BASEPORT	11223
#define MSG		"fubar"

WvX509Mgr *x509;

int tlen=0;
static void do_transfer(WvStream&, void *userdata)
{
    WvSSLStream *ssl = (WvSSLStream *)userdata;
    char buf[1024];
    int rlen = ssl->read(buf, sizeof(buf));
    tlen += rlen;
    printf("Read %d bytes (%d total)\n", rlen, tlen);
}

bool gotmessage = false;
WvSSLStream *singleconn = NULL;
static void getmessage(WvStream&, void *userdata)
{
    WvSSLStream *ssl = (WvSSLStream *)userdata;
    singleconn = ssl;
    WvString msg = ssl->getline(0);
    if (msg == MSG)
	gotmessage = true;
}

static void lcallback(WvStream&, void *userdata)
{
    WvTCPListener *l = (WvTCPListener *)userdata;
    WvTCPConn *conn = l->accept();
    WvSSLStream *ssl = new WvSSLStream(conn, x509, 0, true);
    ssl->setcallback(getmessage, ssl);
    WvIStreamList::globallist.append(ssl, true);
}

WVTEST_MAIN("ssl establish connection")
{
    return; // stupid valgrind

    WvIStreamList::globallist.zap();

    int port = BASEPORT + getpid();

    char hname[32];
    char dname[128];
    gethostname(hname, 32);
    getdomainname(dname, 128);
    WvString fqdn("%s.%s", hname, dname);
    WvString dn("cn=%s,dc=%s", fqdn, dname);
    WvRSAKey *rsa = new WvRSAKey(1024);
    x509 = new WvX509Mgr(dn, rsa);
    WVPASS(x509->test());

    WvString laddrstr("0.0.0.0:%s", port);
    WvIPPortAddr laddr(laddrstr);
    WvTCPListener l(laddr);
    l.setcallback(lcallback, &l);

    WvIStreamList::globallist.append(&l, false);

    printf("Starting listener on %s\n", laddrstr.cstr());

    WVPASS(l.isok());

    WvString caddrstr("127.0.0.1:%s", port);
    WvIPPortAddr caddr(caddrstr);
    WvTCPConn *c = new WvTCPConn(caddr);
    WvSSLStream *ssl = new WvSSLStream(c, NULL);

    WvIStreamList::globallist.append(ssl, true);

    WVPASS(ssl->isok());

    // oh the humanity! i'm not wasting time making this do
    // something more sane though FIXME FIXME FIXME
    printf("This will take 6 seconds waiting for SSL connection\n");
    for (int i=0; i<6; i++)
    {
	if (WvIStreamList::globallist.select(0))
            WvIStreamList::globallist.runonce();
	sleep(1);
    }

    // send a message and ensure it's received
    ssl->print(MSG "\n");
    WvIStreamList::globallist.runonce();
    WVPASS(gotmessage);

    // check for BUGZID:10781
    char data[20000];
    int wlen = ssl->write(data, sizeof(data));

    printf("Wrote %d bytes\n", wlen);

    singleconn->setcallback(do_transfer, singleconn); // setup for next test
    WVPASS(singleconn->select(0));

    // read until we get all the data, or die if there's ever none
    // because it's all there already
    while (tlen < wlen)
    {
        if (WvIStreamList::globallist.select(0))
	    WvIStreamList::globallist.callback();
	else
	    break;
    }

    // if this fails, it's BUGZID:10781
    WVPASS(tlen == wlen);

    delete x509;
    WvIStreamList::globallist.zap();
}

