#include "wvlog.h"
#include "wvtcp.h"
#include "wvsslstream.h"
#include "wvx509.h"
#include "wvstreamlist.h"
#include <signal.h>

volatile bool want_to_die = false;
WvX509Mgr *x509cert;

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
	if (i.ptr() != &s && i->select(0, false, true))
	    i->write(buf, len);
    }
}
 
 
void tcp_incoming(WvStream &_listener, void *userdata)
{
    WvTCPListener *listener = (WvTCPListener *)&_listener;
    WvStreamList *l = (WvStreamList *)userdata;
    WvStream *s = listener->accept();
    
    if (s)
    {
	WvSSLStream *sslsrvr = new WvSSLStream(s, x509cert, false, true);
	l->append(sslsrvr, true, "ss tcp");
	sslsrvr->setcallback(bounce_to_list, l);
    }
}

 
void setupcert()
{
    char hname[32];
    char domname[128];
    char fqdn[160];   
    gethostname(hname,32);
    getdomainname(domname,128);
    strcpy(fqdn,hname);
    strcat(fqdn,".");  
    strcat(fqdn,domname);
    WvString dName("cn=%s,dc=%s",fqdn,domname);
    x509cert = new WvX509Mgr(dName,1024);
    if (x509cert->err)
    {
	fprintf(stderr,"Error: %s\n",(const char *)x509cert->errstr);
	want_to_die = true;
    }	
}


int main(int argc, char **argv)
{
    WvLog log("SSL-Server", WvLog::Info);
    WvStreamList l;
    
    signal(SIGINT,  sighandler_die);
    signal(SIGTERM, sighandler_die);
    signal(SIGPIPE, SIG_IGN);
    
    if (argc >= 2)
    {
	WvString dName = argv[1];
    	x509cert = new WvX509Mgr(dName,1024);
    	if (x509cert->err)
    	{
	    fprintf(stderr,"Error: %s\n",(const char *)x509cert->errstr);
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

