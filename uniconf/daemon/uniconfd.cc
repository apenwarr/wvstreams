#include "wvautoconf.h"
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#ifndef _WIN32
#include <signal.h>
#endif

#include "wvlogrcv.h"
#include "uniconfdaemon.h"
#include "uniclientconn.h"
#include "unisecuregen.h"
#include "unipermgen.h"
#include "wvx509.h"
#include "uniconfroot.h"
#include "wvstrutils.h"
#include "wvfileutils.h"
#include "wvcrash.h"

#ifdef WITH_SLP
#include "slp.h"
#endif

#ifdef _WIN32
#pragma comment(linker, "/include:?UniRegistryGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#pragma comment(linker, "/include:?UniPStoreGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#pragma comment(linker, "/include:?UniIniGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#endif

#define DEFAULT_CONFIG_FILE "ini:uniconf.ini"

static volatile bool want_to_die = false;

#ifndef _WIN32
void signal_handler(int signum)
{
    fprintf(stderr, "\nCaught signal %d; cleaning up and terminating.\n",
	    signum);
    want_to_die = true;
    signal(signum, SIG_DFL);
}
#endif


static void usage(WvStringParm argv0)
{
#ifndef _WIN32
    wverr->print(
	"\n"
        "Usage: %s [-fdVa] [-A moniker] [-p port] [-s sslport] [-u unixsocket] "
		 "<mounts...>\n"
	"     -f   Run in foreground (non-forking)\n"
	"     -d   Print debug messages\n"
        "     -dd  Print lots of debug messages\n"
	"     -V   Print version number and exit\n"
	"     -a   Require authentication on incoming connections\n"
	"     -A   Check all accesses against perms moniker\n"
	"     -p   Listen on given TCP port (default=4111; 0 to disable)\n"
	"     -s   Listen on given TCP/SSL port (default=4112; 0 to disable)\n"
	"     -u   Listen on given Unix socket filename (default=disabled)\n"
	" <mounts> UniConf path=moniker.  eg. \"/foo=ini:/tmp/foo.ini\"\n",
	argv0);
#else
    wverr->print(
	"\n"
	"Usage: %s [-dV] [-p port] [-s sslport] "
		 "<mounts...>\n"
	"     -d   Print debug messages\n"
        "     -dd  Print lots of debug messages\n"
	"     -V   Print version number and exit\n"
	"     -p   Listen on given TCP port (default=4111; 0 to disable)\n"
	"     -s   Listen on given TCP/SSL port (default=4112; 0 to disable)\n"
	" <mounts> UniConf path=moniker.  eg. \"/foo=ini:/tmp/foo.ini\"\n",
	argv0);
#endif
    exit(1);
}

#ifdef WITH_SLP
static void sillyslpcb(SLPHandle hslp, SLPError errcode, void* cookie) 
{ 
    /* return the error code in the cookie */ 
    *(SLPError*)cookie = errcode; 
}
#endif

#ifndef _WIN32
extern char *optarg;
extern int optind;
#else
char *optarg;
int optind = 1;

int getopt(int argc, char *argv[], const char *opts)
{
    static int i = 1;
    for (char *p = argv[optind] + i; optind < argc && argv[optind][0] == '-'; )
    {
	if (!*p)
	{
	    ++optind;
	    i = 1;
	    p = argv[optind] + i;
	    continue;
	}
	char *opt = strchr(opts, *p);
	if (!opt)
	    return -1;
	switch (opt[1])
	{
	case ':':
	    optarg = argv[optind + 1];
	    optind += 2;
	    i = 1;
	    return *p;
	default:
	    optarg = 0;
	    ++i;
	    return *p;
	}
    }
    return -1;
}
#endif

int main(int argc, char **argv)
{
#ifndef _WIN32
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);
#endif
    wvcrash_setup(argv[0]);

    int c, buglevel = 0;
    bool dontfork = false, needauth = false;
    unsigned int port = DEFAULT_UNICONF_DAEMON_TCP_PORT;
    unsigned int sslport = DEFAULT_UNICONF_DAEMON_SSL_PORT;
    WvString unixport, permmon;

    while ((c = getopt(argc, argv, "fdVaA:p:s:u:h?")) >= 0)
    {
	switch (c)
	{
	case 'f':
	    dontfork = true;
	    break;
	    
	case 'd':
	    buglevel++;
	    break;
	    
	case 'V':
	    wverr->print("UniConfDaemon %s\n", WVSTREAMS_RELEASE);
	    exit(0);
	    break;
	    
	case 'a':
	    needauth = true;
	    break;
	    
	case 'A':
	    // needauth = true; // sometimes it makes sense to skip auth...
	    permmon = optarg;
	    break;
	    
	case 'p':
	    port = atoi(optarg);
	    break;
	case 's':
	    sslport = atoi(optarg);
	    break;
	case 'u':
	    unixport = optarg;
	    break;
	    
	case 'h':
	case '?':
	default:
	    usage(argv[0]);
	    break;
	}
    }
    
    // start log output to stderr
    WvLogConsole logcons(2,
			 buglevel >= 2 ? WvLog::Debug5
			 : buglevel == 1 ? WvLog::Debug1
			 : WvLog::Info);
    WvLog log(argv[0], WvLog::Debug);
    
    UniConfRoot cfg;

    for (int i = optind; i < argc; i++)
    {
	WvString path = argv[i], moniker;
	char *cptr = strchr(path.edit(), '=');
	if (!cptr)
	{
	    moniker = path;
	    path = "/";
	}
	else
	{
	    *cptr = 0;
	    moniker = cptr+1;
	}
	
	log("Mounting '%s' on '%s': ", moniker, path);
	IUniConfGen *gen = cfg[path].mount(moniker, false);
	if (gen && gen->isok())
	    log("ok.\n");
	else
	    log("FAILED!\n");
    }
    
    cfg.refresh();
    
    IUniConfGen *permgen = !!permmon ? wvcreate<IUniConfGen>(permmon) : NULL;
    UniConfDaemon daemon(cfg, needauth, permgen);
    WvIStreamList::globallist.append(&daemon, false);

#ifndef _WIN32
    if (!!unixport)
    {
	// FIXME: THIS IS NOT SAFE!
	mkdirp(getdirname(unixport));
	::unlink(unixport);
	if (!daemon.setupunixsocket(unixport))
	    exit(3);
    }
#endif

    if (port && !daemon.setuptcpsocket(WvIPPortAddr("0.0.0.0", port)))
	exit(4);

    if (sslport)
    {
        WvString dName = encode_hostname_as_DN(fqdomainname());
        WvX509Mgr *x509cert = new WvX509Mgr(dName, 1024);
        if (!x509cert->isok())
        {
            log(WvLog::Critical,
		"Couldn't generate X509 certificate: SSL not available.\n");
	    exit(5);
        }
        else if (!daemon.setupsslsocket(
		     WvIPPortAddr("0.0.0.0", sslport), x509cert))
            exit(6);
    }

#ifndef _WIN32
    if (!dontfork)
    {
	// since we're a daemon, we should now background ourselves.
	pid_t pid = fork();
	if (pid > 0) // parent
	    _exit(0);
    }
#endif

    WvString svc, sslsvc;
    
#ifdef WITH_SLP
    // Now that we're this far...
    SLPError slperr; 
    SLPError callbackerr; 
    SLPHandle hslp; 

    slperr = SLPOpen("en", SLP_FALSE, &hslp); 
    if(slperr != SLP_OK)
    { 
        log(WvLog::Critical, "Error opening SLP handle\n"); 
	exit(7);
    } 
    
    // Register UniConf service with SLP 
    if (port)
    {
	svc = WvString("service:uniconf.niti://%s:%s", fqdomainname(), port);
	slperr = SLPReg(hslp, svc, SLP_LIFETIME_MAXIMUM, 0, "", SLP_TRUE, 
			sillyslpcb, &callbackerr);
	
	if(slperr != SLP_OK)
	{ 
	    log(WvLog::Notice, "Error registering UniConf Daemon with SLP Service\n"); 
	    log(WvLog::Notice, "This may be because there is no SLP DA on your network\n");
	}
    }

    // Register UniConf SSL service with SLP 
    if (sslport)
    {
	sslsvc = WvString("service:uniconfs.niti://%s:%s", fqdomainname(), sslport);
	slperr = SLPReg(hslp, sslsvc, SLP_LIFETIME_MAXIMUM, 0, "", SLP_TRUE, 
			sillyslpcb, &callbackerr);
	if(slperr != SLP_OK)
	{ 
	    log(WvLog::Notice, "Error registering UniConf SSL Daemon with SLP Service\n");
	    log(WvLog::Notice, "This may be because there is no SLP DA on your network\n");
	}
    }
#endif
    
    // main loop
    time_t now, last = 0;
    while (!want_to_die && daemon.isok())
    {
	WvIStreamList::globallist.runonce(5000);
	
	// only commit every five seconds or so
	now = time(NULL);
	if (now - last >= 5)
	{
	    cfg.commit();
	    cfg.refresh();
	    if (permgen) permgen->refresh();
	    last = now;
	}
    }
    
    WvIStreamList::globallist.unlink(&daemon);

#ifdef WITH_SLP
    if (!!sslsvc)
	SLPDereg(hslp, sslsvc, sillyslpcb, &callbackerr);
    if (!!svc)
	SLPDereg(hslp, svc, sillyslpcb, &callbackerr);
    
    SLPClose(hslp);
#endif
    
    return 0;
}
