#include "wvargs.h"
#include "wvcrash.h"
#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvsslstream.h"
#include "wvstrutils.h"
#include "wvtcp.h"
#include "wvx509mgr.h"
#include <signal.h>

static bool want_to_die = false;
static const int DEFAULT_KEYSIZE = 1024;


bool validateme(WvX509 *x509)
{
    if (x509 && x509->isok())
    {
    	wvcon->print("X509 Subject: %s\n", x509->get_subject());
    	return true;
    }
    else
    {    
	wvcon->print("X509 Error: %s\n", x509->errstr());
        return false;
    }
}


void sighandler_die(int sig)
{
    fprintf(stderr, "Exiting on signal %d.\n", sig);
    want_to_die = true;
    signal(sig, SIG_DFL);
}


void bounce_to_list(WvStream &s, void *userdata)
{
    char buf[1024];
    size_t len;
    
    len = s.read(buf, sizeof(buf));
    
    WvIStreamList::Iter i(WvIStreamList::globallist);
    for (i.rewind(); i.next(); )
    {
	if (i.ptr() != &s) i->write(buf, len);
    }
}
 
 
void tcp_incoming(WvStream &_listener, void *userdata)
{
    WvTCPListener *listener = (WvTCPListener *)&_listener;
    WvX509Mgr *x509 = (WvX509Mgr *)userdata;
    WvTCPConn *s = listener->accept();
    
    if (s)
    {
	WvSSLStream *sslsrvr = new WvSSLStream(s, x509, validateme,
					       true);
        WvIStreamList::globallist.append(sslsrvr, true, "ss tcp");
	sslsrvr->setcallback(bounce_to_list, NULL);
    }
}

 
int main(int argc, char **argv)
{
    wvcrash_setup(argv[0]);
    
    WvLog log("SSL-Server", WvLog::Info);
    
    WvX509Mgr *x509cert;
    WvString pkcs12file;
    WvString pkcs12pass = "123";

    signal(SIGINT,  sighandler_die);
    signal(SIGTERM, sighandler_die);
    signal(SIGPIPE, SIG_IGN);

    WvArgs args;
    args.add_option('c', "certificate", "Identify self using the specified "
                    "certificate and key pair (in pkcs12 format)", 
                    "FILE", pkcs12file);
    args.add_option('p', "password", "Password for opening the specified "
                    "certificate (if given)", "PASS", pkcs12pass); 
    
    WvStringList remaining_args;
    if (!args.process(argc, argv, &remaining_args))
        return 1;

    if (!!pkcs12file)
    {
        x509cert = new WvX509Mgr;
        x509cert->read_p12(pkcs12file, pkcs12pass);
    }
    else
    {
        WvString dName = encode_hostname_as_DN(fqdomainname());
        x509cert = new WvX509Mgr(dName, DEFAULT_KEYSIZE);
    }
    
    if (!x509cert->isok())
    {
        log("X509 certificate + rsa pair not ok!\n");
        return 1;
    }

    WvTCPListener tcplisten("0.0.0.0:5238");
    tcplisten.setcallback(tcp_incoming, x509cert);
    
    if (!tcplisten.isok())
    {
	log("Can't listen: %s\n", tcplisten.errstr());
	return 1;
    }
    
    log("Listening on %s.\n", *tcplisten.src());
    WvIStreamList::globallist.append(&tcplisten, false, "ss tcp listener"); 
    WvIStreamList::globallist.append(wvcon, false, "wvcon");
    
    wvcon->setcallback(bounce_to_list, NULL);
    
    while (!want_to_die && wvcon->isok() && tcplisten.isok())
        WvIStreamList::globallist.runonce();

    if (!tcplisten.isok())
	wvcon->print("Exited with error: %s\n", tcplisten.errstr());
    

    return 0;
}

