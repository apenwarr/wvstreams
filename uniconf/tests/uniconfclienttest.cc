// Test client for the uniconf daemon

#include <signal.h>
#include <strutils.h>

#include <wvtclstring.h>
#include <wvunixsocket.h>
#include <wvaddr.h>
#include <uniconf.h>
#include <uniconfclient.h>
#include <wvtcp.h>

// Control variable
bool want_to_die = false;

//WvUnixAddr addr("/tmp/uniconf/uniconfsocket");
WvIPPortAddr addr("0.0.0.0", 4111);
void usage()
{
    wvcon->print("uniconfclient usage:  uniconfclient [options]\n");
    wvcon->print("  Where options are:\n");
    wvcon->print("  -h : Display this help message\n");
}

void sighandler_die(int sig)
{
    want_to_die = true;
    signal(sig, SIG_DFL);
}

void errorcheck(WvStream &s)
{
    wvcon->print("Could not create WvUnixConnection.\n");
    wvcon->print("Error number:  %s\n", s.geterr());
    wvcon->print("Error string:  %s\n", s.errstr());
    exit(s.geterr());
}

int received = 0;

void wvconcallback(WvStream &s, void *userdata)
{
    char *line = s.getline(0);
    if (line)
    {
        if (!strcasecmp(line, "quit"))
            want_to_die = true;
    }
}

int main(int argc, char **argv)
{
    UniConf mainconf;
   // WvUnixConn *conn = new WvUnixConn(addr);
    WvTCPConn *conn = new WvTCPConn(addr);
    if (!conn->isok())
    {
        // uhm, why not?
        errorcheck(*conn);
    }

    // Test data setting / retrieval from a mount at /
    {
        UniConf *mounted = &mainconf["/nerf"];
        mounted->generator = new UniConfClient(mounted, conn);
        mounted->generator->load();     // This should do nothing.

        // just test getting a few keys
        {
            UniConf *narf = &mainconf["/nerf/chickens/bob"];
            wvcon->print("/chickens/bob = %s.\n", *narf);
            narf = &mainconf["/wacky\ntest\nsection/  goose  "];
            wvcon->print("/wacky\ntest\nsection/  goose  = %s\n", *narf);
            narf = &mainconf[("/this key should not exist/ bcscso ")];
            wvcon->print("/this key should not exist/ bcscso = %s.\n", *narf);
        }

        // Test getting & setting a key
        {
            UniConf *narf = &mainconf["/chickens/bob"];
            wvcon->print("original:  /chickens/bob = %s.\n", *narf);
            narf->set(wvtcl_escape("Well isn't this just DANDY!"));
            wvcon->print("/chickens/bob = %s.\n", *narf);
        }
        
        mounted->generator->update_tree();
        if (!mounted->child_obsolete)
            wvcon->print("Notifications are not working.\n");
    }

    // Test notification retrieval.
/*    {
        UniConf *mounted = &mainconf["/"];
        mounted->generator = new UniConfClient(mounted,conn);
        mounted->generator->load();
    }*/

    return 0;
}
