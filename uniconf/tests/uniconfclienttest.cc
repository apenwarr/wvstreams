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

void sighandler_die(int sig)
{
    want_to_die = true;
    signal(sig, SIG_DFL);
}

void printheader(WvString header)
{
    wvcon->print("%s\n",header);
    
    for (size_t i = 0; i < header.len(); i++)
        wvcon->print("=");
    wvcon->print("\n\n");
}

void printresult(bool pass, WvString header)
{
    if (pass)
        wvcon->print("\n***** Tests for %s: PASSED *****\n\n", header);
    else
        wvcon->print("\n///// Tests for %s: FAILED /////\n\n", header);
}

bool compareresults(WvString expected, WvString actual)
{
    return expected == actual;
}

bool testgetkeys(UniConf &mainconf, WvString prefix = "")
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

bool testgetfromsections(UniConf &mainconf, WvString prefix = "")
{
    WvString key("%s/chickens", prefix);
    UniConf *neep = &mainconf[key];
    WvString subkey("bob");
    WvString result("goof");
    UniConf *narf = &neep->get(subkey);
    wvcon->print("\"%s/%s\" should be:%s.Real Value:%s.\n", key, subkey, result, *narf );

    return compareresults(result,*narf);
}

bool testgetsetkey(UniConf &mainconf, WvString prefix = "")
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


int main(int argc, char **argv)
{

    system("clear");
    // Test data setting / retrieval from a mount at /

    // just test getting a few keys
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf["/"];
        mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);
        mounted->generator->load();     // This should do nothing.

        printheader("TEST GETTING KEYS WITH MOUNTPOINT /, NO AUTOMOUNT");
        printresult(testgetkeys(mainconf), "GETTING KEYS WITH MOUNTPOINT /, NO AUTOMOUNT");
    }

    // Test getting keys via automount
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf["/"];
        new UniConfClient(mounted, new WvTCPConn(addr), true);

        printheader("TEST GETTING KEYS WITH MOUNTPOINT /, WITH AUTOMOUNT");
        printresult(testgetkeys(mainconf), "GETTING KEYS WITH MOUNTPOINT /, WITH AUTOMOUNT");
    }
    // Test getting a section and then a key
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf["/"];
        mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);
        mounted->generator->load();     // This should do nothing.

        printheader("TEST GETTING FROM A SECTION WITH MOUNTPOINT /, NO AUTOMOUNT");
        printresult(testgetfromsections(mainconf), "GETTING FROM A SECTION WITH MOUNTPOINT /, NO AUTOMOUNT");
    }

    // Test getting a section and then a key via automount
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf["/"];
        new UniConfClient(mounted, new WvTCPConn(addr), true);

        printheader("TEST GETTING FROM A SECTION WITH MOUNTPOINT /, WITH AUTOMOUNT");
        printresult(testgetfromsections(mainconf), "GETTING FROM A SECTION WITH MOUNTPOINT /, WITH AUTOMOUNT");
    }
    // Test getting & setting a key
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf["/"];
        mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);
        mounted->generator->load();     // This should do nothing.

        printheader("TEST SETTING KEYS WITH MOUNTPOINT /, NO AUTOMOUNT");
        printresult(testgetsetkey(mainconf), "SETTING KEYS WITH MOUNTPOINT /, NO AUTOMOUNT");

    }

    // Test getting & setting a key with automount
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf["/"];
        new UniConfClient(mounted, new WvTCPConn(addr), true);

        printheader("TEST SETTING KEYS WITH MOUNTPOINT /, WITH AUTOMOUNT");
        printresult(testgetsetkey(mainconf), "SETTING KEYS WITH MOUNTPOINT /, WITH AUTOMOUNT");

    }
    
    WvString orino("/orino");
    
    // Test getting a key with mount point /orino, no automount
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf[orino];
        mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);
        mounted->generator->load();     // This should do nothing.

        printheader("TEST GETTING KEYS WITH MOUNTPOINT /orino, NO AUTOMOUNT");
        printresult(testgetkeys(mainconf, orino), "GETTING KEYS WITH MOUNTPOINT /orino, NO AUTOMOUNT");
    }

    // Test getting a key with mount point /orino, with automount
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf[orino];
        mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), true);
        mounted->generator->load();     // This should do nothing.

        printheader("TEST GETTING KEYS WITH MOUNTPOINT /orino, WITH AUTOMOUNT");
        printresult(testgetkeys(mainconf, orino), "GETTING KEYS WITH MOUNTPOINT /orino, WITH AUTOMOUNT");
    }
    
    // Test getting a section and then a key with mountpoint /orino
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf[orino];
        mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);
        mounted->generator->load();     // This should do nothing.

        printheader("TEST GETTING FROM A SECTION WITH MOUNTPOINT /orino, NO AUTOMOUNT");
        printresult(testgetfromsections(mainconf), "GETTING FROM A SECTION WITH MOUNTPOINT /orino, NO AUTOMOUNT");
    }

    // Test getting a section and then a key via automount with mountpoint /orino
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf[orino];
        new UniConfClient(mounted, new WvTCPConn(addr), true);

        printheader("TEST GETTING FROM A SECTION WITH MOUNTPOINT /orino, WITH AUTOMOUNT");
        printresult(testgetfromsections(mainconf), "GETTING FROM A SECTION WITH MOUNTPOINT /orino, WITH AUTOMOUNT");
    }
    return 0;
}
