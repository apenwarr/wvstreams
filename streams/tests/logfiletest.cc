#include "wvlogfile.h"
#include <signal.h>

static bool want_to_die = false;

static void sighandler_die(int sig)
{
    want_to_die = true;
    fprintf(stderr,"Exited on Signal: %d\n",sig);
}


int main()
{
    signal(SIGTERM, sighandler_die);
    signal(SIGINT, sighandler_die);
    signal(SIGXFSZ, sighandler_die);

    WvLogFile logger("./logtest", WvLog::Debug5);
    WvLog log("WvLogFile Test", WvLog::Info);

    while(!want_to_die)
    {
	log.print("This is a logging test................................\n");
	log.print("Some more testing.....................................\n");
	log.print("Even more testing.....................................\n");
    }
    return 0;
}