#include <signal.h>

#include "wvcrash.h"
#include "wvlog.h"
#include "wvlogrcv.h"
#include "uniconf.h"
#include "uniconfdaemon.h"
#include "uniclientconn.h"
#include "unisecuregen.h"
#include "unipermgen.h"
#include "wvx509.h"
#include "uniconfroot.h"
#include "strutils.h"

#define DEFAULT_CONFIG_FILE "ini:uniconf.ini"

static UniConfDaemon *globdaemon = NULL;

// we now want execution to stop
static void sighandler_die(int sig)
{
    globdaemon->close();
    signal(sig, SIG_DFL);
}


static void usage()
{
    wverr->print(
        "uniconfdaemon usage:  uniconfdaemon "
            "[-mount mountpoint moniker perms] [-p port] [-ssl sslport] [-d level]\n"
        "    mountpoint - the point to mount the config keys under\n"
        "    moniker    - the moniker, eg. ini:myfile\n"
        "    perms      - moniker to get permissions from (optional)\n"
        "    level      - the debug level\n"
        "                 Critical, Error, Warning, Notice, Info, or Debug[1-5]\n"
        "    port       - the port to listen on for TCP connections (use 0 to disallow)\n"
        "    sslport    - the port to listen on for SSL-encrypted TCP connections (use 0 to disallow)\n");
    exit(2);
}


static WvLog::LogLevel findloglevel(char *arg)
{
    if (!strcasecmp(arg, "Critical"))
        return WvLog::Critical;
    else if (!strcasecmp(arg, "Error"))
        return WvLog::Error;
    else if (!strcasecmp(arg, "Warning"))
        return WvLog::Warning;
    else if (!strcasecmp(arg, "Notice"))
        return WvLog::Notice;
    else if (!strcasecmp(arg, "Info"))
        return WvLog::Info;
    else if (!strcasecmp(arg, "Debug1"))
        return WvLog::Debug1;
    else if (!strcasecmp(arg, "Debug2"))
        return WvLog::Debug2;
    else if (!strcasecmp(arg, "Debug3"))
        return WvLog::Debug3;
    else if (!strcasecmp(arg, "Debug4"))
        return WvLog::Debug4;
    else if (!strcasecmp(arg, "Debug5"))
        return WvLog::Debug5;
    else
        return WvLog::Info;
}


static void trymount(const UniConf &cfg, const UniConfKey &key,
    WvStringParm location, WvStringParm perms = WvString::null)
{
    UniConfGen *gen;
    WvString errormsg;
    if (perms.isnull())
    {
        errormsg = WvString("Unable to mount \"%s\" at \"%s\"\n",
                location, key);
        gen = cfg[key].mount(location);
    }
    else
    {
        errormsg = WvString("Unable to mount \"%s\" at \"%s\","
                "with permissions source \"%s\"\n", 
                location, key, perms);
        gen = cfg[key].mountgen(new UniSecureGen(location,
                    new UniPermGen(perms)));
    }

    if (! gen || ! gen->isok())
    {
        wverr->print(errormsg);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT,  sighandler_die);
    signal(SIGTERM, sighandler_die);
    signal(SIGPIPE, SIG_IGN);
    wvcrash_setup(argv[0]);

    WvLogConsole logcons(2, WvLog::Info);
    
    UniConfRoot root;
    UniConf cfg(root);

    bool mountattempt = false;
    bool needauth = false;
    unsigned int port = DEFAULT_UNICONF_DAEMON_TCP_PORT;
    unsigned int sslport = DEFAULT_UNICONF_DAEMON_SSL_PORT;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i],"-mount"))
        {
            if (argc < i + 3) usage();
            WvString mountpoint = argv[i + 1];
            WvString mountloc = argv[i + 2];
            WvString perms;
            if (argc < i + 4 || argv[i + 3][0] == '-')
                i += 2;
            else
            {
                perms = argv[i + 3];
                i += 3;
                needauth = true;
            }
            trymount(cfg, mountpoint, mountloc, perms);
            mountattempt = true;
        }
        else if (!strcmp(argv[i], "-p"))
        {
            if (argc < i + 2) usage();
            port = WvString(argv[i + 1]).num();
            i += 1;
        }
        else if (!strcmp(argv[i], "-ssl"))
        {
            if (argc < i + 2) usage();
            sslport = WvString(argv[i + 1]).num();
            i += 1;
        }
        else if (!strcmp(argv[i], "-d"))
        {
            if (argc < i + 2) usage();
            logcons.level(findloglevel(argv[i + 1]));
            i += 1;
        }
        else
            usage();
    }

    if (!mountattempt)
        trymount(cfg, UniConfKey::EMPTY, DEFAULT_CONFIG_FILE);

    globdaemon = new UniConfDaemon(cfg, needauth);
    
    // FIXME: THIS IS NOT SAFE!
    system("mkdir -p /tmp/uniconf");
    system("rm -f /tmp/uniconf/uniconfsocket");
    if (! globdaemon->setupunixsocket("/tmp/uniconf/uniconfsocket"))
        exit(1);
    if (port && ! globdaemon->setuptcpsocket(WvIPPortAddr("0.0.0.0", port)))
        exit(1);

    if (sslport)
    {
        WvString dName = encode_hostname_as_DN(fqdomainname());
        WvX509Mgr *x509cert = new WvX509Mgr(dName, 1024);
        if (!x509cert->isok())
        {
            WvLog log("uniconfdaemon", WvLog::Error);
            log("Couldn't generate X509 certificate: SSL not available\n");
        }
        else if (sslport && !globdaemon->setupsslsocket(WvIPPortAddr("0.0.0.0", sslport), x509cert))
            exit(1);
    }
    
    // since we're a daemon, we should now background ourselves.
    pid_t pid = fork();
    if (pid <= 0) // child or failed
    {
	while (globdaemon->isok())
	{
	    if (globdaemon->select(5000))
		globdaemon->callback();
	    else
	    {
		// FIXME: do this *exactly* every so x seconds
		cfg.commit();
		cfg.refresh();
	    }
	}
	globdaemon->close();
	delete globdaemon;
    }
    else // parent
    {
	_exit(0);
    }
    
    return 0;
}
