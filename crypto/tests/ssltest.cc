#include "wvsslstream.h"
#include "wvistreamlist.h"
#include "wvtcp.h"
#include "strutils.h"
#include "wvx509.h"
#include <signal.h>

volatile bool want_to_die = false;

void sighandler_die(int sig)
{
    fprintf(stderr,"Exiting on signal %d.\n", sig);
    want_to_die = true;  
    signal(sig, SIG_DFL);
}


int main(int argc, char **argv)
{
    signal(SIGINT,  sighandler_die);
    signal(SIGTERM, sighandler_die);
    signal(SIGPIPE, SIG_IGN);
    
    WvLog log("ssltest", WvLog::Info);
    log("SSL Test Starting...\n");
    
    // For this test, we default connect to mars's POP3-SSL server...
    WvString target(argc >= 2 ? argv[1] : "mars.net-itech.com:995");
    log("Connecting to %s...\n", target);
    
    // We want to connect with both an anonymous connection, as well
    // as with a certificate.
    WvSSLStream cli(new WvTCPConn(target), NULL);
    
    WvString dName = encode_hostname_as_DN(fqdomainname());
    WvX509Mgr *cert = new WvX509Mgr(dName, 1024);
    WvSSLStream cli2(new WvTCPConn(target), cert);
    
    WvIStreamList::globallist.append(&cli, false);
    WvIStreamList::globallist.append(&cli2, false);
    WvIStreamList::globallist.append(wvin, false);
    
    cli.autoforward(*wvout);
    cli2.autoforward(*wvout);
    wvin->autoforward(cli);
    wvin->autoforward(cli2);
    
    while ((cli.isok() || cli2.isok()) && !want_to_die)
    {
	if (WvIStreamList::globallist.isreadable())
	    WvIStreamList::globallist.runonce();
    }
    
    if (cli.geterr() || cli2.geterr())
	log("Stream closed with error:\ncli: %s\ncli2: %s\n", 
	    cli.errstr(), cli2.errstr());

    log("Done!\n");
    return 0;
}
