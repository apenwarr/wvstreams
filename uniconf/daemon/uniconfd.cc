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
#include <getopt.h>
#include <signal.h>

#define DEFAULT_CONFIG_FILE "ini:uniconf.ini"

static volatile bool want_to_die = false;
void signal_handler(int signum)
{
    fprintf(stderr, "\nCaught signal %d; cleaning up and terminating.\n",
	    signum);
    want_to_die = true;
    signal(signum, SIG_DFL);
}


static void usage(WvStringParm argv0)
{
    wverr->print(
	"\n"
        "Usage: %s [-fdVa] [-A moniker] [-p port] [-s sslport] [-u unixsocket] "
		 "<mounts...>\n"
	"     -f   Run in foreground (non-forking)\n"
	"     -d   Print debug messages\n"
        "     -dd  Print lots of debug messages\n"
	"     -V   Print version number and exit\n"
	"     -a   Require authentication on incoming connections\n"
	"     -A   Require authentication and check perms against moniker\n"
	"     -p   Listen on given TCP port (default=4111; 0 to disable)\n"
	"     -s   Listen on given TCP/SSL port (default=4112; 0 to disable)\n"
	"     -u   Listen on given Unix socket filename (default=disabled)\n"
	" <mounts> UniConf path=moniker.  eg. \"/foo=ini:/tmp/foo.ini\"\n",
	argv0);
    exit(1);
}


extern char *optarg;
extern int optind;

int main(int argc, char **argv)
{
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);
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
	    needauth = true;
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
    
    if (!!unixport)
    {
	// FIXME: THIS IS NOT SAFE!
	mkdirp(getdirname(unixport));
	::unlink(unixport);
	if (!daemon.setupunixsocket(unixport))
	    exit(3);
    }
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

    if (!dontfork)
    {
	// since we're a daemon, we should now background ourselves.
	pid_t pid = fork();
	if (pid > 0) // parent
	    _exit(0);
    }
    
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
    return 0;
}
