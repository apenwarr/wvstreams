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
    wvcon->print("uniconfdaemon usage:  uniconfdaemon [-mount mode file mount-point] [-d level]\n");
    wvcon->print("    Where:        mode is one of:  ini\n");
    wvcon->print("    file:         is the file name to mount\n");
    wvcon->print("    mount-point:  is the point to mount the config keys under\n");
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
        if (!strcmp(argv[i],"-mount"))
        {
            WvString *mode = new WvString(argv[i+1]);
            WvString *location = new WvString(argv[i+2]);
            WvString *mp = new WvString(argv[i+3]);

            if (!*mode || !*location || !*mp)
            {
                delete mode;
                delete location;
                delete mp;
                usage();
            }
            
            strings.append(mode, true);
            strings.append(location, true);
            strings.append(mp, true);
            i += 3;
        }
        else if (!strcmp(argv[i], "-d"))
        {
            level = findloglevel(argv[++i]);
        }
        else
            usage();
    }
   
    daem = new UniConfDaemon(level);
    WvStringList::Iter i(strings);
    i.rewind();

    while (i.next())
    {
        WvString mode = i();
        i.next();
        WvString location = i();
        i.next();
        WvString mp = i();
        daem->domount(mode, location, mp);
    }
    daem->run();
    
    return 0;
}
