#include "uniconfroot.h"
#include "unireadonlygen.h"
#include "unicachegen.h"
#include "unitempgen.h"
#include "unilistgen.h"
#include "uniinigen.h"

#include "uniconfdaemon.h"

#include "wvistreamlist.h"
#include "wvfile.h"
#include "wvtest.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>

#define UNICONFD_SOCK "/tmp/unitempgen-uniconfd"
#define UNICONFD_INI "/tmp/unitempgen-uniconfd.ini"

// write out a temporary ini file for use, saves flushing entries
bool write_ini()
{
    WvFile outfile(UNICONFD_INI, O_CREAT | O_WRONLY | O_TRUNC);
    if (outfile.isok())
    {
        outfile.print("%s\n%s\n", "[eth0]", "dhcpd = 1");
        return true;
    }

    return false;
}

WVTEST_MAIN("tempgen/cachgen basics")
{
    signal(SIGPIPE, SIG_IGN);

    if (!write_ini())
    {
        WVFAIL(true || "Could not write ini file");
        exit(1); 
    }

    pid_t cfg_handler = fork();
    if (!cfg_handler) // child only
    {
        UniIniGen *unigen = new UniIniGen(UNICONFD_INI, 0666);
        UniTempGen *defgen = new UniTempGen();
        UniConfGenList *l = new UniConfGenList(); 

        // Must addRef here since both ReadOnlyGen and mountgen take ownership.
        defgen->addRef();
        l->append(unigen, true);
        l->append(new UniReadOnlyGen(defgen), true);

        UniListGen *listgen = new UniListGen(l);

        UniConfRoot uniconf;
        uniconf["cfg"].mountgen(listgen);
        uniconf["default"].mountgen(defgen);
        uniconf["auth"].mount("temp:");
        uniconf["tmp"].mount("temp:");

        UniConf defcfg(uniconf["default"]);

        UniConfDaemon daemon(uniconf, false, NULL);
        daemon.setupunixsocket(UNICONFD_SOCK, 0777);

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
        UniConfRoot cfg_ok("retry:unix:" UNICONFD_SOCK);
        for (;;)
        {
            cfg_ok.xset("/tmp/dummy", "foo");
            if (cfg_ok.xget("/tmp/dummy") == "foo") break;
            sleep(1);
        }
    }

    /* Setup subtree root */
    UniConfRoot cfg("cache:subtree:unix:" UNICONFD_SOCK " cfg");
    
    /* test to see that value gets pushed into the tempgen as well */
    int initial_value = cfg["eth0"].xgetint("dhcpd", 0);
    cfg["eth0"].xsetint("dhcpd", !initial_value);
    int new_value = cfg["eth0"].xgetint("dhcpd", 0);

    WVPASS(initial_value != new_value);
   
    // kill off the daemon
    kill(cfg_handler, 15);
    unlink(UNICONFD_INI);
    unlink(UNICONFD_SOCK);
}
