#include "uniconfroot.h"
#include "unireadonlygen.h"
#include "unicachegen.h"
#include "unitempgen.h"
#include "unilistgen.h"
#include "uniinigen.h"

#include "uniconfdaemon.h"

#include "wvistreamlist.h"
#include "wvfile.h"
#include "wvunixsocket.h"
#include "wvtest.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>

// write out a temporary ini file for use, saves flushing entries
static bool write_ini(WvString &ininame)
{
    int fd;
    ininame = "/tmp/iniXXXXXX";
    if ((fd = mkstemp(ininame.edit())) == (-1))
        return false;    
    close(fd);


    WvFile outfile(ininame, O_CREAT | O_WRONLY | O_TRUNC);
    if (outfile.isok())
    {
        outfile.print("%s\n%s\n", "[eth0]", "dhcpd = 1");
        return true;
    }

    return false;
}

static WvString get_sockname()
{
    int fd;
    WvString sockname = "/tmp/sockXXXXXX";
    if ((fd = mkstemp(sockname.edit())) == (-1))
        return "";    
    close(fd);

    return sockname;
}

WVTEST_MAIN("tempgen/cachegen basics")
{
    signal(SIGPIPE, SIG_IGN);

    WvString ininame;
    if (!write_ini(ininame))
    {
        WVFAIL(true || "Could not write ini file");
        exit(1); 
    }

    WvString sockname = get_sockname();
    if (!sockname)
    {
        WVFAIL(true || "Could not get socket filename");
        exit(1); 
    }

    pid_t cfg_handler = fork();
    if (!cfg_handler) // child only
    {
        UniIniGen *unigen = new UniIniGen(ininame, 0666);

        UniConfRoot uniconf;
        uniconf["cfg"].mountgen(unigen);
        uniconf["tmp"].mount("temp:");

        UniConf defcfg(uniconf["default"]);

        UniConfDaemon daemon(uniconf, false, NULL);
        daemon.setupunixsocket(sockname, 0777);

        WvIStreamList::globallist.append(&daemon, false);
        while (daemon.isok())
        {
            WvIStreamList::globallist.runonce(5000);
            uniconf.commit();
        }
        WVFAIL("uniconfd exited unexpectedly");
    }

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

    WVPASS(initial_value != new_value);
   
    // kill off the daemon
    kill(cfg_handler, 15);
    unlink(ininame.cstr());
    unlink(sockname.cstr());
}


WVTEST_MAIN("cache:subtree:unix assertion failure")
{
    signal(SIGPIPE, SIG_IGN);

    WvString ininame;
    if (!write_ini(ininame))
    {
        WVFAIL(true || "Could not write ini file");
        exit(1); 
    }

    WvString sockname = get_sockname();
    if (!sockname)
    {
        WVFAIL(true || "Could not get socket filename");
        exit(1); 
    }

    pid_t cfg_handler = fork();
    if (!cfg_handler) // child only
    {
        UniIniGen *unigen = new UniIniGen(ininame, 0666);

        UniConfRoot uniconf;
        uniconf["cfg"].mountgen(unigen);
        uniconf["tmp"].mount("temp:");

        UniConf defcfg(uniconf["default"]);

        UniConfDaemon daemon(uniconf, false, NULL);
        daemon.setupunixsocket(sockname, 0777);

        WvIStreamList::globallist.append(&daemon, false);
        while (daemon.isok())
        {
            WvIStreamList::globallist.runonce(5000);
            uniconf.commit();
        }
        WVFAIL("uniconfd exited unexpectedly");
    }

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
   
    // kill off the daemon
    kill(cfg_handler, 15);
    unlink(ininame.cstr());
    unlink(sockname.cstr());
}
