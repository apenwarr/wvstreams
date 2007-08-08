#include "uniconfroot.h"
#include "unireadonlygen.h"
#include "unicachegen.h"
#include "unitempgen.h"
#include "unilistgen.h"
#include "uniinigen.h"
#include "uniconfgen-sanitytest.h"

#include "wvistreamlist.h"
#include "wvfile.h"
#include "wvunixsocket.h"
#include "wvtest.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>


// write out a temporary ini file for use, saves flushing entries
static void write_ini(WvString &ininame)
{
    ininame = WvString("/tmp/unitempgenvsdaemonini-%s", getpid());

    WvFile outfile(ininame, O_CREAT | O_RDWR | O_TRUNC);
    outfile.print("%s\n%s\n", "[eth0]", "dhcpd = 1");
    outfile.close();
}

WVTEST_MAIN("tempgen/cachegen basics")
{
    signal(SIGPIPE, SIG_IGN);

    WvString ininame;
    write_ini(ininame);

    WvString sockname = WvString("/tmp/unitempgensock-%s", getpid());
    
    struct stat sock_exist;
    if (stat(sockname, &sock_exist) == 0)
    {
        WVFAIL(true || "Socket filename already exists");
        unlink(ininame);
        exit(1); 
    }

    UniConfTestDaemon daemon(sockname, WvString("ini:%s", ininame));

    // Wait for child to become responsive
    {
        WvString root("retry:unix:%s", sockname);
        UniConfRoot cfg_ok(root);
        for (;;)
        {
            cfg_ok.xset("/tmp/dummy", "foo");
            if (cfg_ok.xget("/tmp/dummy") == "foo") break;
            sleep(1);
        }
    }

    /* Setup subtree root */
    WvString root("subtree:unix:%s cfg", sockname);
    UniConfRoot cfg(root);
    
    int initial_value = cfg["eth0"].xgetint("dhcpd", 0);
    cfg["eth0"].xsetint("dhcpd", !initial_value);
    cfg.commit();
    int new_value = cfg["eth0"].xgetint("dhcpd", 0);

    WVFAILEQ(initial_value, new_value);
   
    unlink(ininame);
}


WVTEST_MAIN("cache:subtree:unix assertion failure")
{
    signal(SIGPIPE, SIG_IGN);

    WvString ininame;
    write_ini(ininame);

    WvString sockname = WvString("/tmp/unitempgensock2-%s", getpid());

    UniConfTestDaemon daemon(sockname, WvString("ini:%s", ininame));

    // Wait for child to become responsive
    {
        WvString root("retry:unix:%s", sockname);
        UniConfRoot cfg_ok(root);
        for (;;)
        {
            cfg_ok.xset("/tmp/dummy", "foo");
            if (cfg_ok.xget("/tmp/dummy") == "foo") break;
            sleep(1);
        }
    }

    /* Setup subtree root */
    WvString root("cache:subtree:unix:%s cfg", sockname);
    UniConfRoot cfg(root);
    
    WvUnixConn unixconn(sockname);
    WVPASS(unixconn.isok());

    unlink(ininame);
}
