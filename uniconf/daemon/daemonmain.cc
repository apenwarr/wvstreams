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
            "[-mount mountpoint moniker] [-perms moniker] [-p port] [-ssl sslport] [-d level]\n"
        "    mountpoint - the point to mount the config keys under\n"
        "    moniker    - the moniker, eg. ini:myfile\n"
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
    WvStringParm location, WvStringParm perms)
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

WvString hostname()
{
    int maxlen = 0;
    for(;;)
    {
        maxlen += 80;
        char *name = new char[maxlen];
        int result = gethostname(name, maxlen);
        if (result == 0)
        {
            WvString hostname(name);
            delete [] name;
            return hostname;
        }
        assert(errno == EINVAL);
    }
}

WvString domainname()
{
    int maxlen = 0;
    for(;;)
    {
        maxlen += 128;
        char *name = new char[maxlen];
        int result = getdomainname(name, maxlen);
        if (result == 0)
        {
            WvString domainname(name);
            delete [] name;
            return domainname;
        }
        assert(errno == EINVAL);
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

    UniConfKey mountpoint = UniConfKey::EMPTY;
    WvString mountloc = DEFAULT_CONFIG_FILE;
    WvString perms;
    unsigned int port = DEFAULT_UNICONF_DAEMON_TCP_PORT;
    unsigned int sslport = DEFAULT_UNICONF_DAEMON_SSL_PORT;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i],"-mount"))
        {
            if (argc < i + 3) usage();
            mountpoint = argv[i + 1];
            mountloc = argv[i + 2];
            i += 2;
        }
        else if (!strcmp(argv[i], "-perms"))
        {
            if (argc < i + 2) usage();
            perms = argv[i + 1];
            i += 1;
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

    trymount(cfg, mountpoint, mountloc, perms);

    globdaemon = new UniConfDaemon(cfg, !perms.isnull());
    
    // FIXME: THIS IS NOT SAFE!
    system("mkdir -p /tmp/uniconf");
    system("rm -f /tmp/uniconf/uniconfsocket");
    if (! globdaemon->setupunixsocket("/tmp/uniconf/uniconfsocket"))
        exit(1);
    if (port && ! globdaemon->setuptcpsocket(WvIPPortAddr("0.0.0.0", port)))
        exit(1);

    if (sslport)
    {
        // FIXME: copied from sslsrvtest.cc, this looks too simple
        WvString hname = hostname();
        WvString domname = domainname();
        WvString fqdn("%s.%s", hname, domname);
        WvString dName("cn=%s,dc=%s",fqdn,domname);
        WvX509Mgr *x509cert = new WvX509Mgr(dName,1024);
        if (!x509cert->isok())
        {
            WvLog log("uniconfdaemon", WvLog::Error);
            log("Couldn't generate X509 certificate: SSL not available\n");
        }
        else if (sslport && !globdaemon->setupsslsocket(WvIPPortAddr("0.0.0.0", sslport), x509cert))
            exit(1);
    }
    
    while (globdaemon->isok())
    {
        if (globdaemon->select(-1))
            globdaemon->callback();
    }
    globdaemon->close();
    delete globdaemon;
    return 0;
}
