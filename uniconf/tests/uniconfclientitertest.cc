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

int main(int argc, char **argv)
{
    // Iterator spam stuff

    // Test a normal iterator
    {
        UniConf mainconf;
        UniConf *mounted = &mainconf["/"];
        mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);//conn);
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
        mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);//conn);
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
        mounted->generator = new UniConfClient(mounted, new WvTCPConn(addr), false);//conn);
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

    return 0;
}
