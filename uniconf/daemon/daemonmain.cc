#include "uniconfdaemon.h"
#include <signal.h>
#include "wvcrash.h"
#include "wvlog.h"

UniConfDaemon *daem;
// we now want execution to stop
void sighandler_die(int sig)
{
    daem->log(WvLog::Info, "Dying on signal %s\n", sig);    
    daem->want_to_die = true;
    signal(sig, SIG_DFL);
}

void usage()
{
    wvcon->print("uniconfdaemon usage:  uniconfdaemon "
        "[-mount mountpoint moniker] [-d level]\n");
    wvcon->print("    mountpoint:   is the point to mount the config keys under\n");
    wvcon->print("    moniker:      is the moniker, eg. ini://myfile\n");
    wvcon->print("    level:        is one of:  Critical, Error, Warning, Notice, Info, or Debug[1-5]\n");

    exit(2);
}

WvLog::LogLevel findloglevel(char *arg)
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

int main(int argc, char **argv)
{
    signal(SIGINT,  sighandler_die);
    signal(SIGTERM, sighandler_die);
    signal(SIGPIPE, SIG_IGN);
    wvcrash_setup(argv[0]);

    WvStringList strings;
    WvLog::LogLevel level = WvLog::Info;
    
    for (int i=1; i < argc; i++)
    {
        if (!strcmp(argv[i],"-mount") && i + 2 < argc)
        {
            WvString *mp = new WvString(argv[i + 1]);
            WvString *location = new WvString(argv[i + 2]);

            strings.append(mp, true);
            strings.append(location, true);
            i += 2;
        }
        else if (!strcmp(argv[i], "-d"))
        {
            level = findloglevel(argv[i + 1]);
            i += 1;
        }
        else
        {
            usage();
            return 1;
        }
    }
   
    daem = new UniConfDaemon(level);

    if (strings.isempty())
        daem->domountdefault();

    WvStringList::Iter i(strings);
    i.rewind();

    while (i.next())
    {
        UniConfKey mp(i());
        i.next();
        UniConfLocation location(i());
        daem->domount(mp, location);
    }
    daem->run();
    
    return 0;
}
