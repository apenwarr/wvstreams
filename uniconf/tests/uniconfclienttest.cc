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

WvString printheader(WvString h, WvString mountpoint, bool automount)
{
    WvString header("%s WITH MOUNTPOINT %s, %s", h, mountpoint, (automount ? "WITH AUTOMOUNT" : "NO AUTOMOUNT"));
    wvcon->print("%s\n",header);
    
    for (size_t i = 0; i < header.len(); i++)
        wvcon->print("=");
    wvcon->print("\n\n");
    return header;
}

void printresult(bool pass, WvString header)
{
    if (pass)
        wvcon->print("\n***** %s: PASSED *****\n\n", header);
    else
        wvcon->print("\n///// %s: FAILED /////\n\n", header);
}

bool compareresults(WvString expected, WvString actual)
{
    return expected == actual;
}

bool testgetkeys(UniConf &mainconf, WvString prefix)
{
    bool pass = true;


    WvString key("%s/chickens/bob", prefix);
    WvString result("goof");
    UniConf *narf = &mainconf[key];
    
    pass &= compareresults(result,*narf);
    
    wvcon->print("\"%s\" should be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
    key = WvString("%s/wacky\ntest\nsection/  goose  ", prefix);
    result = "bluebayou";
    narf = &mainconf[key];

    pass &= compareresults(result,*narf);
    
    wvcon->print("\"%s\" should be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
    key = WvString("%s/this key should not exist/ bcscso ", prefix);
    narf = &mainconf[key];
    result = WvString();
    
    pass &= compareresults(result,*narf);
    
    wvcon->print("\"%s\" should be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));

    return pass;
 
}

bool testgetfromsections(UniConf &mainconf, WvString prefix)
{
    WvString key("%s/chickens", prefix);
    UniConf *neep = &mainconf[key];
    WvString subkey("bob");
    WvString result("goof");
    UniConf *narf = &neep->get(subkey);
    wvcon->print("\"%s/%s\" should be:%s.Real Value:%s.\n", key, subkey, result, *narf );

    return compareresults(result,*narf);
}

bool testgetsetkey(UniConf &mainconf, WvString prefix)
{
    bool pass = true;
    
    WvString key("%s/chickens/bob", prefix);
    WvString result("goof");
    UniConf *narf = &mainconf[key];

    pass &= compareresults(result,*narf);
    
    wvcon->print("\"%s\" should be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
    result = "Well isn't this just DANDY!";
    narf->set(result);

    pass &= compareresults(result,*narf);
    
    wvcon->print("\"%s\" should now be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));
    result = "goof";
    narf->set(wvtcl_escape(result));
    wvcon->print("\"%s\" should now be:%s.Is it?  %s.\n", key, result, (result == *narf ? "Yes" : "No"));

    pass &= compareresults(result,*narf);

    return pass;
}

void usage()
{
    wvcon->print("uniconfclienttest usage:\n");
    wvcon->print("uniconfclienttest [-h] [-t test_type]\n");
    wvcon->print("\t-h - display this message\n");
    wvcon->print("\t-t test_type:  test_type is one of all, get, set or section\n");
    exit(0);
}

int main(int argc, char **argv)
{

    system("clear");
    
    WvString mountpoint("/");
    WvString h;

    WvString totest = "all";

    if (argc == 2 && !strcasecmp(argv[1], "-h"))
    {
        usage();
    }
    else if (3 == argc)
    {
        if (!strcasecmp("-t",argv[1]))
            totest = argv[2]; 
    }

    bool automount = true;
    for (int i = 0; i < 2; i++)
    {
        // just test getting a few keys
        if ("all" == totest || "get" == totest)
        {
            do
            {
                automount = !automount;
                UniConf mainconf;
                UniConf *mounted = &mainconf[mountpoint];
                mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), automount);
                mounted->generator->load();     // This should do nothing.

                h = printheader("TEST GETTING KEYS", mountpoint, automount);
                printresult(testgetkeys(mainconf, mountpoint), h);
            }while(!automount);

            // Test getting keys via automount
        }
        // Test getting a section and then a key
        if ("all" == totest || "section" == totest) 
        {
            do
            {
                automount = !automount;
                UniConf mainconf;
                UniConf *mounted = &mainconf[mountpoint];
                mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), automount);
                mounted->generator->load();     // This should do nothing.

                h = printheader("TEST GETTING FROM A SECTION", mountpoint, automount);
                printresult(testgetfromsections(mainconf,mountpoint), h);
            }while(!automount);

            // Test getting a section and then a key via automount
        }
        // Test getting & setting a key
        
        if ("all" == totest || "set" == totest) 
        {
            do
            {
                automount = !automount;
                UniConf mainconf;
                UniConf *mounted = &mainconf[mountpoint];
                mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), automount);
                mounted->generator->load();     // This should do nothing.

                h = printheader("TEST SETTING KEYS", mountpoint, automount);
                printresult(testgetsetkey(mainconf,mountpoint), h);

            }while(!automount);

            // Test getting & setting a key with automount
        }
        mountpoint = "/orino";
    }
    
    return 0;
}
