
#include <signal.h>

#include "wvcrash.h"
#include "wvlog.h"
#include "wvlogrcv.h"
#include "uniconf.h"
#include "uniconfdaemon.h"
#include "uniconfconn.h"

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
            "[-mount mountpoint moniker] [-d level]\n"
        "    mountpoint - the point to mount the config keys under\n"
        "    moniker    - the moniker, eg. ini:myfile\n"
        "    level      - the debug level\n"
        "                 Critical, Error, Warning, Notice, Info, or Debug[1-5]\n");
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
    WvStringParm location)
{
    UniConfGen *gen = cfg[key].mount(location);
    if (! gen || ! gen->isok())
    {
        wverr->print("Unable to mount at \"%s\" from \"%s\"\n",
            key, location);
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
    
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i],"-mount") && i + 2 < argc)
        {
            if (argc < i + 3) usage();
            mountattempt = true;
            trymount(cfg, argv[i + 1], argv[i + 2]);
            i += 2;
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
    if (! mountattempt)
        trymount(cfg, UniConfKey::EMPTY, DEFAULT_CONFIG_FILE);

    globdaemon = new UniConfDaemon(cfg);
    
    // FIXME: THIS IS NOT SAFE!
    system("mkdir -p /tmp/uniconf");
    system("rm -f /tmp/uniconf/uniconfsocket");
    if (! globdaemon->setupunixsocket("/tmp/uniconf/uniconfsocket"))
        exit(1);
    if (! globdaemon->setuptcpsocket(WvIPPortAddr("0.0.0.0",
        DEFAULT_UNICONF_DAEMON_TCP_PORT)))
        exit(1);
    
    while (globdaemon->isok())
    {
        if (globdaemon->select(1000))
            globdaemon->callback();
    }
    globdaemon->close();
    delete globdaemon;
    return 0;
}
