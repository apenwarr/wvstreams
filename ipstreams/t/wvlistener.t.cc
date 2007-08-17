#include "wvlistener.h"
#include "wvtest.h"
#include "wvtcp.h"
#include "wvistreamlist.h"

class XListener : public WvListener
{
public:
    WvTCPListener *l;
    
    XListener(const WvIPPortAddr &_listenport)
	: WvListener(l = new WvTCPListener(_listenport))
    {
    }
    
    IWvStream *accept()
    {
	return l->accept();
    }
};


static IWvStream *mystream;

static void acceptor(IWvStream *s, void *)
{
    mystream = s;
}


WVTEST_MAIN("wvlistener")
{
    XListener l("");
    const WvAddr *listenport = l.src();
    printf("Listening on %s\n", ((WvString)*listenport).cstr());
    
    l.onaccept(acceptor);
    
    WvIStreamList::globallist.append(&l, false);
    WvTCPConn tcp(*listenport);
    WvIStreamList::globallist.append(&tcp, false);
    
    while (tcp.isok() && !mystream)
	WvIStreamList::globallist.runonce(-1);
    WVPASS(tcp.isok());
    WVPASS(mystream);
    if (!mystream)
    {
	WVPASS(mystream->isok());
	mystream->write("text!\n", 6);
	WVPASSEQ(tcp.getline(-1), "text!");
	delete mystream;
    }
}

