// Test client for uniconf integers 

/**
 * FIXME: This test case does not test integers, or set up/tear down its
 * environment explicatly 
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


bool check(WvStringParm test, int value, int expected)
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


bool testgetabool(const UniConf &mainconf, WvStringParm key, bool expected)
{
    UniConf conf(mainconf["bools"][key]);
    return check(conf.fullkey(), conf.getint(), expected);
}

bool testgetbools(const UniConf &mainconf)
{
    bool pass = true;

    pass = testgetabool(mainconf, "true", true) && pass;
    pass = testgetabool(mainconf, "yes", true) && pass;
    pass = testgetabool(mainconf, "on", true) && pass;
    pass = testgetabool(mainconf, "enabled", true) && pass;
    pass = testgetabool(mainconf, "false", false) && pass;
    pass = testgetabool(mainconf, "no", false) && pass;
    pass = testgetabool(mainconf, "off", false) && pass;
    pass = testgetabool(mainconf, "disabled", false) && pass;
    
    return pass;
}

bool testgetanint(const UniConf &mainconf, WvStringParm key, int expected)
{
    UniConf conf(mainconf["ints"][key]);
    return check(conf.fullkey(), conf.getint(), expected);
}

bool testgetints(const UniConf &mainconf)
{
    bool pass = true;

    pass = testgetanint(mainconf, "foo", 0) && pass;
    pass = testgetanint(mainconf, "bar", 1) && pass;
    pass = testgetanint(mainconf, "baz", -1) && pass;
    pass = testgetanint(mainconf, "boffle", 42) && pass;
    pass = testgetanint(mainconf, "bluh", 0) && pass;
    
    return pass;
}


void usage(const char *name)
{
    wvcon->print("%s usage:\n", name);
    wvcon->print("%s [-h]\n", name);
    wvcon->print("\t-h - display this message\n");
    exit(0);
}

int main(int argc, char **argv)
{
    WvString location("ini:inttest.ini");
    WvString mountpoint("");
    UniConfRoot root;
    UniConf mounted(root[mountpoint]);
    mounted.mount(location);

    printheader("TEST GETTING BOOLS", mountpoint);
    printresult(testgetbools(mounted));

    printheader("TEST GETTING INTS", mountpoint);
    printresult(testgetints(mounted));

    return 0;
}
