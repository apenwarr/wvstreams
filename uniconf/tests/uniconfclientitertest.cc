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

void printheader(WvString h, WvString mountpoint, bool automount)
{
    WvString header("%s WITH MOUNTPOINT %s, %s", h, mountpoint, (automount ? "WITH AUTOMOUNT" : "NO AUTOMOUNT"));
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
        wvcon->print("Key:%s has value:%s.\n", i->name, *i);
    }
}

void testrecursiveiter(UniConf &mainconf)
{
    UniConf *nerf = &mainconf["/"];
    UniConf::Iter i(*nerf);
    for (i.rewind(); i.next();)
    {
        wvcon->print("Key:%s has value:%s.\n", i->name, *i);
    }
}
void testxiter(UniConf &mainconf)
{
    UniConf *nerf = &mainconf["/"];
    UniConf::XIter i(*nerf, "/*/chickens");
    for (i.rewind(); i._next();)
    {
        wvcon->print("Key:%s has value:%s.\n", i->name, *i);
    }
}
int main(int argc, char **argv)
{
    WvString mountpoint("/");
    bool automount = true;
    for (int i = 0; i < 2; i++)
    {

        // Test a normal iterator
        do
        {
            automount = !automount;
            UniConf mainconf;
            UniConf *mounted = &mainconf[mountpoint];
            mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), automount);
            if (!automount)
                mounted->generator->load();     // This should do nothing.

            printheader("TEST NORMAL ITERATORS", mountpoint, automount);
            testnormaliter(mainconf);
        }while(!automount);

        // Test a recursive iterator
        do
        {
            automount = !automount;
            UniConf mainconf;
            UniConf *mounted = &mainconf["/"];
            mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);
            if (!automount)
                mounted->generator->load();     // This should do nothing.
            
            printheader("TEST RECURSIVE ITERATORS", mountpoint, automount);
            testrecursiveiter(mainconf);
        }
        while(!automount);

        // Test an XIter
        do
        {
            automount = !automount;
            
            UniConf mainconf;
            UniConf *mounted = &mainconf["/"];
            mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);//conn);
            mounted->generator->load();     // This should do nothing.

            printheader("TEST X ITERATORS", mountpoint, automount);
            testxiter(mainconf);

        }
        while(!automount);


        // Test a recursive iterator
        {
            UniConf mainconf;
            UniConf *temp = &mainconf["/"];
            temp->generator = new UniConfClient(temp, new WvTCPConn(WvIPPortAddr("192.168.12.26", 4111)), false);//conn);
            UniConf *mounted = &mainconf["/orino"];
            mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);//conn);
            mounted->generator->load();     // This should do nothing.

            wvcon->print("\n================================\n");
            wvcon->print("|   TEST RECURSIVE ITERATORS   |\n");
            wvcon->print("================================\n\n");
            UniConf *nerf = &mainconf["/"];
            UniConf::RecursiveIter i(*nerf);
            for (i.rewind(); i._next();)
            {
                wvcon->print("Key:%s has value:%s.\n", i->name, *i);
            }
        }
    }
    return 0;
}
