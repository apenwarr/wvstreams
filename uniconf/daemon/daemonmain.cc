#include "uniconfdaemon.h"

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
    wvcon->print("uniconfdaemon usage:  uniconfdaemon [mode file mount-point]\n");
    wvcon->print("    Where:        mode is one of:  ini\n");
    wvcon->print("    file:         is the file name to mount\n");
    wvcon->print("    mount-point:  is the point to mount the config keys under\n");

    exit(2);
}

int main(int argc, char **argv)
{
    signal(SIGINT,  sighandler_die);
    signal(SIGTERM, sighandler_die);

    daem = new UniConfDaemon();

    if (argc == 4)
    {
       daem->domount(argv[1], argv[2], argv[3]);
    }
    else if (argc != 1)
        usage();
    
    daem->run();
    
    return 0;
}
