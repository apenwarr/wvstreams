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

    UniConfConnFactory *fctry = NULL;
    
    // Via TCP Connection factory
    {
        fctry = new UniConfTCPFactory(addr);

        // Test data setting / retrieval from a mount at /

        // just test getting a few keys
        {
            UniConf mainconf;
            UniConf *mounted = &mainconf["/"];
            mounted->generator = new UniConfClient(mounted, fctry);//conn);
            mounted->generator->load();     // This should do nothing.

            wvcon->print("=========================\n");
            wvcon->print("|   TEST GETTING KEYS   |\n");
            wvcon->print("=========================\n\n");
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

        // Test getting a section and then a key
        {
            UniConf mainconf;
            UniConf *mounted = &mainconf["/"];
            mounted->generator = new UniConfClient(mounted, fctry);//conn);
            mounted->generator->load();     // This should do nothing.

            wvcon->print("=======================================\n");
            wvcon->print("|   TEST GETTING KEYS FROM A SECTION  |\n");
            wvcon->print("=======================================\n\n");
            WvString key("/chickens");
            UniConf *neep = &mainconf[key];
            WvString subkey("bob");
            WvString result("goof");
            UniConf *narf = &neep->get(subkey);
            wvcon->print("\"%s/%s\" should be:%s.Is it?  %s.\n", key, subkey, result, (result == *narf ? "Yes" : "No"));
        }

        // Test getting & setting a key
        {
            UniConf mainconf;
            UniConf *mounted = &mainconf["/"];
            mounted->generator = new UniConfClient(mounted, fctry);//conn);
            mounted->generator->load();     // This should do nothing.

            wvcon->print("\n=========================\n");
            wvcon->print("|   TEST SETTING KEYS   |\n");
            wvcon->print("=========================\n\n");
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

        // Test a normal iterator
        {
            UniConf mainconf;
            UniConf *mounted = &mainconf["/"];
            mounted->generator = new UniConfClient(mounted, fctry);//conn);
            mounted->generator->load();     // This should do nothing.

            wvcon->print("\n=============================\n");
            wvcon->print("|   TEST NORMAL ITERATORS   |\n");
            wvcon->print("=============================\n\n");
            UniConf *nerf = &mainconf["/"];
            UniConf::Iter i(*nerf);
            for (i.rewind(); i.next();)
            {
                wvcon->print("Key:%s has value:%s.\n", i->name, *i);
            }
        }

        // Test a recursive iterator
        {
            UniConf mainconf;
            UniConf *mounted = &mainconf["/"];
            mounted->generator = new UniConfClient(mounted, fctry);//conn);
            mounted->generator->load();     // This should do nothing.

            wvcon->print("\n================================\n");
            wvcon->print("|   TEST RECURSIVE ITERATORS   |\n");
            wvcon->print("================================\n\n");
            UniConf *nerf = &mainconf["/"];
            UniConf::RecursiveIter i(*nerf);
            for (i.rewind(); i._next();)
            {
                wvcon->print("Key:%s has value:%s.\n", i->gen_full_key(), *i);
            }
        }

       // Test an XIter
        {
            UniConf mainconf;
            UniConf *mounted = &mainconf["/"];
            mounted->generator = new UniConfClient(mounted, fctry);//conn);
            mounted->generator->load();     // This should do nothing.

            wvcon->print("\n========================\n");
            wvcon->print("|   TEST X ITERATORS   |\n");
            wvcon->print("========================\n\n");
            UniConf *nerf = &mainconf["/"];
            UniConf::XIter i(*nerf, "/");
            for (i.rewind(); i._next();)
            {
                wvcon->print("Key:%s has value:%s.\n", i->name, *i);
            }
        }
    }
    return 0;
}
