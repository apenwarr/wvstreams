#include "wvsslstream.h"
#include "wvstreamlist.h"
#include "wvtcp.h"
#include <signal.h>

volatile bool want_to_die;


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
    
    // For this test, we connect to bond.net's POP3-SSL server...
    WvString target(argc >= 2 ? argv[1] : "mail.bond.net:995");
    log("Connecting to %s...\n", target);
    WvSSLStream cli(new WvTCPConn(target),NULL,true);
    WvStreamList l;
    
    l.append(&cli, false);
    l.append(wvcon, false);
    
    cli.autoforward(*wvcon);
    wvcon->autoforward(cli);
    
    while (cli.isok() && wvcon->isok() && !want_to_die)
    {
	if (l.select(-1))
	    l.callback();
    }
    
    if (cli.geterr())
	log("Stream closed with error: %s\n", cli.errstr());
    
    log("Done!\n");
    return 0;
}
