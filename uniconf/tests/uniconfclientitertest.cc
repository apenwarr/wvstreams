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

void printheader(WvString h, WvString mountpoint)
{
    WvString header("%s WITH MOUNTPOINT %s", h, mountpoint);
    wvcon->print("%s\n",header);
    
    for (size_t i = 0; i < header.len(); i++)
        wvcon->print("=");
    wvcon->print("\n\n");
}

void testnormaliter(UniConf &mainconf)
{
    UniConf *nerf = &mainconf["/"];
    UniConf::Iter i(*nerf);
    for (i.rewind(); i.next();)
    {
        wvcon->print("Key:%s has value:%s.\n",
            i().full_key(), i->value());
    }
}

void testrecursiveiter(UniConf &mainconf)
{
    UniConf *nerf = &mainconf["/"];
    UniConf::RecursiveIter i(*nerf);
    for (i.rewind(); i.next();)
    {
        wvcon->print("Key:%s has value:%s.\n",
            i().full_key(), i->value());
    }
}
#if 0
void testxiter(UniConf &mainconf)
{
    UniConf *nerf = &mainconf["/"];
    UniConf::XIter i(*nerf, "/*/chickens");
    for (i.rewind(); i._next();)
    {
        wvcon->print("Key:%s has value:%s.\n", i().full_key(), *i);
    }
}
#endif

int main(int argc, char **argv)
{
    WvString mountpoint("/");
    UniConfLocation location("tcp://");
    
    for (int i = 0; i < 2; i++)
    {

        {
        // Test a normal iterator
            UniConf mainconf;
            UniConf *mounted = &mainconf[mountpoint];
            mounted->mount(location);

            printheader("TEST NORMAL ITERATORS", mountpoint);
            testnormaliter(mainconf);
        }

        {
        // Test a recursive iterator
            UniConf mainconf;
            UniConf *mounted = &mainconf[mountpoint];
            mounted->mount(location);
            
            printheader("TEST RECURSIVE ITERATORS", mountpoint);
            testrecursiveiter(mainconf);
        }
#if 0
        // Test an XIter
        
        {
            UniConf mainconf;
            UniConf *mounted = &mainconf[mountpoint];
            mounted->mount(location);
            printheader("TEST X ITERATORS", mountpoint);
            testxiter(mainconf);
        }
#endif
        mountpoint = "/orino";
    }
    return 0;
}
