// Test client for the uniconf daemon

/**
 * FIXME: This test case is not as useful at it could be because:
 *   1) it only tests a very small subset of features
 *   2) it does not setup / teardown its environment explicitly
 */

#include <signal.h>
#include <strutils.h>

#include "wvtclstring.h"
#include "wvunixsocket.h"
#include "wvaddr.h"
#include "uniconf.h"
#include "uniclientgen.h"
#include "wvtcp.h"

void printheader(WvString h, WvString mountpoint)
{
    WvString header("%s WITH MOUNTPOINT %s", h, mountpoint);
    wvcon->print("%s\n",header);
    
    for (size_t i = 0; i < header.len(); i++)
        wvcon->print("=");
    wvcon->print("\n\n");
}

void printresult(bool pass)
{
    if (pass)
        wvcon->print("\n***** PASSED *****\n\n");
    else
        wvcon->print("\n///// FAILED /////\n\n");
}


bool check(WvString test, WvString value, WvString expected)
{
    if (value == expected)
    {
        wvcon->print("OK - %s: got \"%s\"\n", test, value);
        return true;
    }
    else
    {
        wvcon->print("FAIL - %s: expected \"%s\", got \"%s\"\n",
            test, expected, value);
        return false;
    }
}


bool testgetkeys(const UniConf &mainconf)
{
    bool pass = true;

    UniConf narf(mainconf["chickens/bob"]);
    pass = check(narf.fullkey(), narf.get(), "goof") && pass;
  
    narf = mainconf["wacky test section/  goose  "];
    pass = check(narf.fullkey(), narf.get(), "weasels") && pass;

    narf = mainconf["this key should not exist/ bcscso "];
    pass = check(narf.fullkey(), narf.get(), WvString::null) && pass;
    
    return pass;
}

bool testgetfromsections(const UniConf &mainconf)
{
    bool pass = true;

    UniConf neep(mainconf["chickens"]);
    UniConf sub(neep["bob"]);
    pass = check(sub.fullkey(), sub.get(), "goof") && pass;

    neep = mainconf["users"];
    sub = neep["apenwarr"];
    UniConf subsub(sub["ftp"]);
    pass = check(subsub.fullkey(), subsub.get(), "1") && pass;

    subsub = sub["pptp"];
    pass = check(subsub.fullkey(), subsub.get(), "0") && pass;

    return pass;
}

bool testgetsetkey(const UniConf &mainconf)
{
    bool pass = true;

    UniConf narf(mainconf["chickens/bob"]);
    pass = check(narf.fullkey(), narf.get(), "goof") && pass;

    narf.set("troop");
    pass = check(narf.fullkey(), narf.get(), "troop") && pass;

    narf.remove();
    pass = check(narf.fullkey(), narf.get(), WvString::null) && pass;

#if 1
    // FIXME: UniConf daemon handling of empty string broken
    narf.set("");
    pass = check(narf.fullkey(), narf.get(), "") && pass;
#endif

    narf.set("goof");
    pass = check(narf.fullkey(), narf.get(), "goof") && pass;
    
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
//    system("clear");
    
    WvString location("tcp:localhost:4111");
    WvString mountpoint("");
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

    for (int i = 0; i < 2; i++)
    {
        // Test getting a few keys
        if ("all" == totest || "get" == totest)
        {
            UniConfRoot root;
            UniConf mainconf(root);
            UniConf mounted(mainconf[mountpoint]);
            mounted.mount(location);

            printheader("TEST GETTING KEYS", mountpoint);
            printresult(testgetkeys(mounted));
        }
        
        // Test getting a section and then a key
        if ("all" == totest || "section" == totest) 
        {
            UniConfRoot root;
            UniConf mainconf(root);
            UniConf mounted(mainconf[mountpoint]);
            mounted.mount(location);

            printheader("TEST GETTING FROM A SECTION", mountpoint);
            printresult(testgetfromsections(mounted));
        }
        
        // Test getting & setting a key
        if ("all" == totest || "set" == totest) 
        {
            UniConfRoot root;
            UniConf mainconf(root);
            UniConf mounted(mainconf[mountpoint]);
            mounted.mount(location);

            printheader("TEST SETTING KEYS", mountpoint);
            printresult(testgetsetkey(mounted));
        }

        mountpoint = "orino";
    }
    
/*    WvTCPConn conn(addr);
    conn.select(0, true,true,false);
    conn.print("|******************************|\n");
*/    
    return 0;
}
