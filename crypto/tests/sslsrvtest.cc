#include "wvlog.h"
#include "wvtcp.h"
#include "wvsslstream.h"
#include "wvx509.h"
#include "wvstreamlist.h"
#include "strutils.h"
#include "wvcrash.h"
#include <signal.h>

volatile bool want_to_die = false;
WvX509Mgr *x509cert = NULL;

void sighandler_die(int sig)
{
    fprintf(stderr, "Exiting on signal %d.\n", sig);
    want_to_die = true;
    signal(sig, SIG_DFL);
}


void bounce_to_list(WvStream &s, void *_list)
{
    char buf[1024];
    size_t len;
    
    len = s.read(buf, sizeof(buf));
    
    WvStreamList &list = *(WvStreamList *)_list;
    WvStreamList::Iter i(list);
    for (i.rewind(); i.next(); )
    {
	if (i.ptr() != &s) i->write(buf, len);
    }
}
 
 
void tcp_incoming(WvStream &_listener, void *userdata)
{
    WvTCPListener *listener = (WvTCPListener *)&_listener;
    WvStreamList *l = (WvStreamList *)userdata;
    WvTCPConn *s = listener->accept();
    
    if (s)
    {
	assert(x509cert);
	WvSSLStream *sslsrvr = new WvSSLStream(s, x509cert, false, true);
	l->append(sslsrvr, true, "ss tcp");
	sslsrvr->setcallback(bounce_to_list, l);
    }
}

 
void setupcert()
{
    WvString fqdn;
    WvString hname = hostname();
    WvString domname = domainname();
    if (!!domname)
        fqdn = WvString("%s.%s", hname, domname);
    else
	fqdn = hname;      
    
    WvString dName = encode_hostname_as_DN(fqdn);
    x509cert = new WvX509Mgr(dName, 1024);
    if (!x509cert->isok())
    {
	wverr->print("Error: %s\n", x509cert->errstr());
	want_to_die = true;
    }
}


int main(int argc, char **argv)
{
    // Set up WvCrash
    wvcrash_setup(argv[0]);
    
    // make sure electric fence works
    free(malloc(1));

    WvLog log("SSL-Server", WvLog::Info);
    WvStreamList l;
    
    signal(SIGINT,  sighandler_die);
    signal(SIGTERM, sighandler_die);
    signal(SIGPIPE, SIG_IGN);
    
    if (argc >= 2)
    {
	WvString dName = argv[1];
    	x509cert = new WvX509Mgr(dName, 1024);
    	if (!x509cert->isok())
    	{
	    wverr->print("Error: %s\n", x509cert->errstr());
	    want_to_die = true;
    	}	
    } 
    else
    {
	setupcert();
    }

    WvTCPListener tcplisten("0.0.0.0:5238");
    tcplisten.setcallback(tcp_incoming, &l);
    
    if (!tcplisten.isok())
    {
	log("Can't listen: %s\n", tcplisten.errstr());
	return 1;
    }
    
    log("Listening on %s.\n", *tcplisten.src());
    l.append(&tcplisten, false, "ss tcp listener"); 
    l.append(wvcon, false);
    
    wvcon->setcallback(bounce_to_list, &l);
    
    while (!want_to_die && wvcon->isok() && tcplisten.isok())
    {
	if (l.select(-1))
	    l.callback(); 
    }
     
    return 0;
}

