// Test client for the uniconf daemon

#include <signal.h>
#include <strutils.h>

#include "wvtclstring.h"
#include "wvunixsocket.h"
#include "wvaddr.h"
#include "uniconf.h"
#include "uniconfclient.h"
#include "wvtcp.h"

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

    UniConfConnFactory *fctry = NULL;
    
    // Via TCP Connection factory
    {
        fctry = new UniConfTCPFactory(addr);

        // Test data setting / retrieval from a mount at /
        {
            UniConf *mounted = &mainconf["/"];
            mounted->generator = new UniConfClient(mounted, fctry);//conn);
            mounted->generator->load();     // This should do nothing.

            // just test getting a few keys
            {
                WvString key("/chickens/bob");
                WvString result("goof");
                UniConf *narf = &mainconf[key];
                wvcon->print("\"%s\" should be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
                key = "/wacky\ntest\nsection/  goose  ";
                result = "bluebayou";
                narf = &mainconf[key];
                wvcon->print("\"%s\" should be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
                key = "/this key should not exist/ bcscso ";
                narf = &mainconf[key];
                result = WvString();
                wvcon->print("\"%s\" should be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
            }

            // Test getting & setting a key
            {
                WvString key("/chickens/bob");
                WvString result("goof");
                UniConf *narf = &mainconf[key];
                wvcon->print("\"%s\" should be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
                result = "Well isn't this just DANDY!";
                narf->set(result);
                wvcon->print("\"%s\" should now be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
                result = "goof";
                narf->set(wvtcl_escape(result));
                wvcon->print("\"%s\" should now be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
            }

            // Test getting a subtree
            {
                UniConf *nerf = &mainconf["/"];
                UniConf::Iter i(*nerf);
                for (i.rewind(); i.next();)
                {
                    wvcon->print("Key:%s has value:%s.\n", i->name, *i);
                }
            }

            mounted->save();
        }

    }
    return 0;
}
