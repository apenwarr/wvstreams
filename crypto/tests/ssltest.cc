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
    WvString pkcs12file, pkcs12pass;
    WvX509Mgr *x509 = NULL;

    WvArgs args;
    args.add_option('c', "certificate", "Identify self using the specified "
                    "certificate and key pair (in pkcs12 format)", 
                    "FILE", pkcs12file);
    args.add_option('p', "password", "Password for opening the specified "
                    "certificate (if given)", "PASS", pkcs12pass); 
    args.add_required_arg("HOST:PORT", false);
    
    WvStringList remaining_args;
    if (!args.process(argc, argv, &remaining_args))
        return 1;
    
    WvString target = remaining_args.popstr();

    if (!!pkcs12file)
    {
        x509 = new WvX509Mgr;
        x509->read_p12(pkcs12file, pkcs12pass);
        
        if (!x509->isok())
        {
            log(WvLog::Error, "Couldn't load certificate! (did you specify a password?)\n");
            return 1;
        }
    }

    log("Connecting to %s...\n", target);

    WvSSLStream *cli = new WvSSLStream(new WvTCPConn(target), x509);

    WvIStreamList::globallist.append(cli, false, "client");
    WvIStreamList::globallist.append(wvin, false, "wvin");
    
    cli->autoforward(*wvout);
    wvin->autoforward(*cli);
    log("Oh dear.\n");
    
    while (cli->isok() && !want_to_die)
	WvIStreamList::globallist.runonce();
    
    if (cli->geterr())
	log("Stream closed with error: %s\n", cli->errstr());

    log("Done!\n");
    WVRELEASE(cli);
    WVRELEASE(x509);

    return 0;
}
