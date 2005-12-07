#include "wvsslstream.h"
#include "wvistreamlist.h"
#include "wvtcp.h"
#include "strutils.h"
#include "wvx509.h"
#include "wvargs.h"
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

    WvArgs args;
    args.add_optional_arg("HOST:PORT", false);
    
    WvStringList remaining_args;
    if (!args.process(argc, argv, &remaining_args))
        return 1;
    
    // For this test, we default connect to mars's POP3-SSL server...
    WvString target;
    if (!(target = remaining_args.popstr()))
        target = "mars.net-itech.com:995";
    log("Connecting to %s...\n", target);
    
    WvSSLStream cli(new WvTCPConn(target), NULL);
    
    WvIStreamList::globallist.append(&cli, false, "client");
    WvIStreamList::globallist.append(wvin, false, "wvin");
    
    cli.autoforward(*wvout);
    wvin->autoforward(cli);
    
    while (cli.isok() && !want_to_die)
	WvIStreamList::globallist.runonce();
    
    if (cli.geterr())
	log("Stream closed with error: %s\n", cli.errstr());

    log("Done!\n");
    return 0;
}
