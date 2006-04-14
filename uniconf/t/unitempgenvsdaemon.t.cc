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

    pid_t cfg_handler = fork();
    if (!cfg_handler) // child only
    {
        UniIniGen *unigen = new UniIniGen(ininame, 0666);

        UniConfRoot uniconf;
        uniconf["cfg"].mountgen(unigen);
        uniconf["tmp"].mount("temp:");

        UniConfDaemon daemon(uniconf, false, NULL);
        daemon.setupunixsocket(sockname, 0777);

        WvIStreamList::globallist.append(&daemon, false, "daemon");
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
   
    // kill off the daemon and clean up the zombie
    kill(cfg_handler, 15);

    // In case a signal is in the process of being delivered..
    pid_t rv;
    while ((rv = waitpid(cfg_handler, NULL, 0)) != cfg_handler)
        if (rv == -1 && errno != EINTR)
            break;
    WVPASSEQ(rv, cfg_handler);

    unlink(ininame);
    unlink(sockname);
}


WVTEST_MAIN("cache:subtree:unix assertion failure")
{
    signal(SIGPIPE, SIG_IGN);

    WvString ininame;
    write_ini(ininame);

    WvString sockname = WvString("/tmp/unitempgensock2-%s", getpid());

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

        WvIStreamList::globallist.append(&daemon, false, "daemon");
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

    // kill off the daemon and clean up the zombie
    kill(cfg_handler, 15);

    // In case a signal is in the process of being delivered...
    pid_t rv;
    while ((rv = waitpid(cfg_handler, NULL, 0)) != cfg_handler)
        if (rv == -1 && errno != EINTR)
            break;
    WVPASSEQ(rv, cfg_handler);

    unlink(ininame);
    unlink(sockname);
}
