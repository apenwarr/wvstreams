// Test client for the uniconf daemon

/**
 * FIXME: This test case is not as useful at it could be because:
 *   1) it only tests a very small subset of features
 *   2) it does not setup / teardown its environment explicitly
 *   3) it's a mess... there's only about 20 useful lines
 *      of code in here, but many are duplicated
 */

#include <signal.h>
#include <strutils.h>

#include "wvtclstring.h"
#include "wvunixsocket.h"
#include "wvaddr.h"
#include "uniconf.h"
#include "uniconfclient.h"
#include "wvtcp.h"

WvString printheader(WvString h, WvString mountpoint)
{
    WvString header("%s WITH MOUNTPOINT %s", h, mountpoint);
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
    
    pass &= compareresults(result, narf->value());
    
    wvcon->print("\"%s\" should be:%s.Is it?  %s.  Real Value: %s\n",
        key, result, (result == narf->value() ? "Yes" : "No"),
        narf->value());
    key = WvString("%s/wacky test section/  goose  ", prefix);
    result = "bluebayou";
    narf = &mainconf[key];

    pass &= compareresults(result, narf->value());
    
    wvcon->print("\"%s\" should be:%s.Is it?  %s.  Real Value: %s\n",
        key, result, (result == narf->value() ? "Yes" : "No"),
        narf->value());
    key = WvString("%s/this key should not exist/ bcscso ", prefix);
    narf = &mainconf[key];
    result = WvString();
    
    pass &= compareresults(result, narf->value());
    
    wvcon->print("\"%s\" should be:%s.Is it?  %s.\n",
        key, result, (result == narf->value() ? "Yes" : "No"));

    return pass;
 
}

bool testgetfromsections(UniConf &mainconf, WvString prefix)
{
    bool pass = true;
    WvString key("%s/chickens", prefix);
    UniConf *neep = & mainconf[key];
    WvString subkey("bob");
    WvString result("goof");
    UniConf *narf = & (*neep)[subkey];
    pass &= compareresults(result, narf->value());
    wvcon->print("\"%s/%s\" should be:%s.Real Value:%s.\n",
        key, subkey, result, narf->value());

    key = WvString("%s/users", prefix);
    neep = &mainconf[key];
    subkey = "apenwarr";
    WvString sub_subkey("ftp");
    result = "1";
    narf = & (*neep)[subkey];
    narf = & (*narf)[sub_subkey];
    pass &= compareresults(result, narf->value());
    wvcon->print("\"%s/%s/%s\" should be:%s.Real Value:%s.\n",
        key, subkey, sub_subkey, result, narf->value());

    sub_subkey = "pptp";
    result = "0";
    narf = &mainconf[key][subkey][sub_subkey];
    pass &= compareresults(result, narf->value());
    wvcon->print("\"%s/%s/%s\" should be:%s.Real Value:%s.\n",
        key, subkey, sub_subkey, result, narf->value());

    return pass;
}

bool testgetsetkey(UniConf &mainconf, WvString prefix)
{
    bool pass = true;
    
    WvString key("%s/chickens/bob", prefix);
    WvString result("goof");
    UniConf *narf = & mainconf[key];

    pass &= compareresults(result, narf->value());
    
    wvcon->print("\"%s\" should be:%s.Is it?  %s.\n",
        key, result, (result == narf->value() ? "Yes" : "No"));
    result = "Well isn't this just DANDY!";
    narf->set(UniConfKey::EMPTY, result);

    pass &= compareresults(result, narf->value());
    
    wvcon->print("\"%s\" should now be:%s.Is it?  %s.\n",
        key, result, (result == narf->value() ? "Yes" : "No"));
    result = "goof";
    narf->set(UniConfKey::EMPTY, wvtcl_escape(result));
    wvcon->print("\"%s\" should now be:%s.Is it?  %s.\n",
        key, result, (result == narf->value() ? "Yes" : "No"));

    pass &= compareresults(result, narf->value());

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
    
    UniConfLocation location("tcp://");
    WvString mountpoint("");
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

    for (int i = 0; i < 2; i++)
    {
        // just test getting a few keys
        if ("all" == totest || "get" == totest)
        {
            UniConf mainconf;
            UniConf *mounted = & mainconf[mountpoint];
            mounted->mount(location);

            h = printheader("TEST GETTING KEYS", mountpoint);
            printresult(testgetkeys(mainconf, mountpoint), h);

        }
        // Test getting a section and then a key
        if ("all" == totest || "section" == totest) 
        {
            UniConf mainconf;
            UniConf *mounted = & mainconf[mountpoint];
            mounted->mount(location);

            h = printheader("TEST GETTING FROM A SECTION", mountpoint);
            printresult(testgetfromsections(mainconf, mountpoint), h);
        }
        // Test getting & setting a key
        
        if ("all" == totest || "set" == totest) 
        {
                UniConf mainconf;
                UniConf *mounted = & mainconf[mountpoint];
                mounted->mount(location);

                h = printheader("TEST SETTING KEYS", mountpoint);
                printresult(testgetsetkey(mainconf, mountpoint), h);

        }
    }
/*    WvTCPConn conn(addr);
    conn.select(0, true,true,false);
    conn.print("|******************************|\n");
*/    
    return 0;
}
