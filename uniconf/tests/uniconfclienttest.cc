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

void incomingcallback(WvStream &s, void *userdata)
{
    int len = -1;
    WvBuffer *buf = (WvBuffer *)userdata;
    char *cptr[1024];
    
    while (len != 0 && s.select(0, true, false, false))
    {
        len = s.read(cptr, 1023);
        cptr[len] ='\0';
        buf->put(cptr, len);
    }

    WvString *line = wvtcl_getword(*buf, "\n");
    while (line)
    {
        WvBuffer foo;
        foo.put(*line);

        // get the command
        WvString *cmd = wvtcl_getword(foo);
        WvString *key = wvtcl_getword(foo);
        while(cmd && key)
        {
            // check the command
            if (*cmd == "RETN") // value returned
            {
                WvString *value = wvtcl_getword(foo);
                wvcon->print("Received value %s for key %s.\n", *value, *key);
                received++;
            }
            else
            {
                wvcon->print("Received command:  %s and key: %s.\n", *cmd, *key);
            }

            // get a new command & key
            cmd = wvtcl_getword(foo);
            key = wvtcl_getword(foo);
            
            // We don't need to unget here, since if we broke on a \n,
            // that means that we were at the end of a word, and since all
            // requests are "single line" via tclstrings, no worries.
        }
        line = wvtcl_getword(*buf, "\n");
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
    UniConf *mounted = &mainconf["/"];
    mounted->generator = new UniConfClient(mounted, conn);
    mounted->generator->load();
    // just test getting a key
    {
        UniConf *narf = &mainconf["/chickens/bob"];
        wvcon->print("/chickens/bob = %s.\n", *narf);
        narf = &mainconf["/wacky\ntest\nsection/  goose  "];
        wvcon->print("/wacky\ntest\nsection/  goose  = %s\n", *narf);
    }

    // Test getting & setting a key
    {
        UniConf *narf = &mainconf["/chickens/bob"];
        wvcon->print("/chickens/bob = %s.\n", *narf);
        narf->set(wvtcl_escape("Well isn't this just DANDY!"));
        wvcon->print("/chickens/bob = %s.\n", *narf);
        if (mainconf.dirty || mainconf.child_dirty)
            mainconf.save();
    }

//    conn.print("quit\n");
    return 0;
}
