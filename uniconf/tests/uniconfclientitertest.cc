// Test client for the uniconf daemon

#include <signal.h>
#include <strutils.h>

#include "wvtclstring.h"
#include "wvunixsocket.h"
#include "wvaddr.h"
#include "uniconf.h"
#include "uniconfclient.h"
#include "wvtcp.h"
#include "wvlogrcv.h"

void printheader(WvString h, WvString mountpoint)
{
    WvString header("\n\n%s WITH MOUNTPOINT %s", h, mountpoint);
    wvcon->print("%s\n",header);
    
    for (size_t i = 0; i < header.len(); i++)
        wvcon->print("=");
    wvcon->print("\n\n");
}


void testnormaliter(const UniConf &mainconf)
{
    UniConf::Iter i(mainconf);
    for (i.rewind(); i.next();)
    {
        wvcon->print("Key:%s has value:%s.\n",
            i->fullkey(), i->get());
    }
}


void testrecursiveiter(const UniConf &mainconf)
{
    UniConf::RecursiveIter i(mainconf);
    for (i.rewind(); i.next();)
    {
        wvcon->print("Key:%s has value:%s.\n",
            i->fullkey(), i->get());
    }
}


int main(int argc, char **argv)
{
    WvString mountpoint("/");
    WvString location("tcp:localhost:4111");
    WvLogConsole cons(2, WvLog::Debug4);
    
    for (int i = 0; i < 2; i++)
    {
        // Test a normal iterator
        {
            UniConfRoot root;
            UniConf mainconf(root);
            UniConf mounted(mainconf[mountpoint]);
            mounted.mount(location);

            printheader("TEST NORMAL ITERATORS", mountpoint);
            testnormaliter(mainconf);
        }

        // Test a recursive iterator
        {
            UniConfRoot root;
            UniConf mainconf(root);
            UniConf mounted(mainconf[mountpoint]);
            mounted.mount(location);
            
            printheader("TEST RECURSIVE ITERATORS", mountpoint);
            testrecursiveiter(mainconf);
        }
        mountpoint = "/orino";
    }
    return 0;
}
