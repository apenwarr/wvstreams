#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "unireplicategen.h"
#include "wvistreamlist.h"
#include "wvtimeutils.h"
#include "uniconfgen-sanitytest.h"

#include <sys/types.h>
#include <sys/wait.h>

#ifdef MACOS
#include <signal.h>
#else
#include <sys/signal.h>
#endif

WVTEST_MAIN("UniReplicateGen Sanity Test")
{
    UniReplicateGen *gen = new UniReplicateGen();
    gen->append(new UniTempGen(), true);
    UniConfGenSanityTester::sanity_test(gen, "replicate:temp: temp:");
    WVRELEASE(gen);
}

WVTEST_MAIN("basic")
{
    UniConfRoot cfg("replicate:temp: temp:");
    WVFAIL(cfg.haschildren());
    WVPASS(cfg["/key"].getme().isnull());
    
    cfg["/key"].setme("value");
    WVPASS(cfg.haschildren());
    WVPASS(cfg["/key"].getme() == "value");
    
    cfg["/key"].setme(WvString::null);
    WVFAIL(cfg.haschildren());
    WVPASS(cfg["/key"].getme().isnull());
}

WVTEST_MAIN("propagation")
{
    UniTempGen tmps[2];
    tmps[0].set("/key0", "value0");
    tmps[0].set("/key", "value0");
    tmps[1].set("/key1", "value1");
    tmps[1].set("/key", "value1");
    
    UniReplicateGen rep;
    
    rep.append(&tmps[1], false);
    WVPASS(!rep.exists("/key0"));
    WVPASS(rep.get("/key1") == "value1");
    WVPASS(rep.get("/key") == "value1");
    WVPASS(!tmps[0].exists("/key1"));
    WVPASS(tmps[0].get("/key") == "value0");
    WVPASS(!tmps[1].exists("/key0"));
    WVPASS(tmps[1].get("/key") == "value1");
   
    rep.prepend(&tmps[0], false);
    WVPASS(rep.get("/key0") == "value0");
    WVPASS(rep.get("/key1") == "value1");
    WVPASS(rep.get("/key") == "value0");
    WVPASS(tmps[0].get("/key1") == "value1");
    WVPASS(tmps[0].get("/key") == "value0");
    WVPASS(tmps[1].get("/key0") == "value0");
    WVPASS(tmps[1].get("/key") == "value0");
    
    rep.set("key", "value");
    WVPASS(rep.get("key") == "value");
    WVPASS(tmps[0].get("key") == "value");
    WVPASS(tmps[1].get("key") == "value");
    
    tmps[0].set("key", "value0");
    WVPASS(rep.get("key") == "value0");
    WVPASS(tmps[0].get("key") == "value0");
    WVPASS(tmps[1].get("key") == "value0");
    
    tmps[1].set("key", "value1");
    WVPASS(rep.get("key") == "value1");
    WVPASS(tmps[0].get("key") == "value1");
    WVPASS(tmps[1].get("key") == "value1");
}

static int callback_count = 0;

static void callback(const UniConf &uniconf, const UniConfKey &key)
{
    ++callback_count;
}

static void kill_and_harvest(const pid_t pid)
{
    // Never, ever try to kill pid 0 or -1.
    if (pid <= 0)
        return;

    kill(pid, 15);
    pid_t rv;
    while ((rv = waitpid(pid, NULL, 0)) != pid)
    {
        // in case a signal is in the process of being delivered..
        if (rv == -1 && errno != EINTR)
            break;
    }
    WVPASS(rv == pid);
}

WVTEST_MAIN("retry:uniconfd")
{
    signal(SIGPIPE, SIG_IGN);

    WvString uniconfd_sock("/tmp/unireplicategen-uniconfd-%s", getpid());
    WvString uniconfd_ini("/tmp/unireplicategen-uniconfd-%s.ini", getpid());
    WvString ini_moniker("ini:%s", uniconfd_ini);
    WvString l_moniker("unix:%s", uniconfd_sock);
    const char *uniconfd_argv[] =
        {
            "uniconfd",
            "-l", l_moniker,
            ini_moniker,
            NULL
        };
    pid_t uniconfd_pid;

    unlink(uniconfd_ini.cstr());
        
    UniConfRoot cfg(WvString("replicate:{retry:unix:%s 100} temp:", 
                             uniconfd_sock));
    cfg.add_callback(&callback_count, "/", callback, true);
    WVPASS(callback_count == 0);

    int old_callback_count;
   
    old_callback_count = callback_count;
    cfg["/key"].setme("value");
    WVPASS(cfg["/key"].getme() == "value");
    WVPASS(callback_count > old_callback_count);

    // FIXME: This is a lot more complicated than using the UniConfTestDaemon
    // class.  However, using it breaks the unit tests a tiny bit and I don't
    // have time to figure it out.  But whatever you do, don't copy this code!
    // Use the UniConfTestDaemon!
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", (char *const *)uniconfd_argv);
    	_exit(1);
    }
    wvdelay(100);
    
    // wait for connect
    {
    	UniConfRoot another_cfg(WvString("retry:unix:%s", uniconfd_sock));
    
        for (;;)
        {
            another_cfg.xset("wait", "ping");
            if (another_cfg.xget("wait") == "ping") break;
            wvdelay(100);
        }
    }

    WVPASS(cfg["/key"].getme() == "value");

    old_callback_count = callback_count;
    {
    	UniConfRoot another_cfg(WvString("unix:%s", uniconfd_sock));

        WVPASS(another_cfg["/key"].getme() == "value");

    	another_cfg["/key"].setme("value one");
    	WVPASS(another_cfg["/key"].getme() == "value one");
    }
    WVPASS(cfg["/key"].getme() == "value one");
    WVPASS(callback_count > old_callback_count);
   
    kill_and_harvest(uniconfd_pid);
    unlink(uniconfd_sock.cstr());
    
    WVPASS(cfg["/key"].getme() == "value one");

    old_callback_count = callback_count;
    cfg["/key"].setme("value two");
    WVPASS(cfg["/key"].getme() == "value two");
    WVPASS(callback_count > old_callback_count);
    
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", (char *const *)uniconfd_argv);
    	_exit(1);
    }
    wvdelay(100);
    
    // wait for connect
    {
    	UniConfRoot another_cfg(WvString("retry:unix:%s", uniconfd_sock));
    
        for (;;)
        {
            another_cfg.xset("wait", "pong");
            if (another_cfg.xget("wait") == "pong") break;
            wvdelay(100);
        }
    }

    old_callback_count = callback_count;
    WVPASS(cfg["/key"].getme() == "value one");
    WVPASS(callback_count > old_callback_count);
    
    old_callback_count = callback_count;
    {
    	UniConfRoot another_cfg(WvString("unix:%s", uniconfd_sock));
    
    	another_cfg["/key"].setme("value three");
    	WVPASS(another_cfg["/key"].getme() == "value three");
    }
    WVPASS(cfg["/key"].getme() == "value three");
    WVPASS(callback_count > old_callback_count);
    
    old_callback_count = callback_count;
    cfg["/key"].setme("value four");
    WVPASS(cfg["/key"].getme() == "value four");
    WVPASS(callback_count > old_callback_count);

    {
    	UniConfRoot another_cfg(WvString("unix:%s", uniconfd_sock));
    
    	WVPASS(another_cfg["/key"].getme() == "value four");
    }

    kill_and_harvest(uniconfd_pid);

    WVPASS(cfg["/key"].getme() == "value four");

    unlink(uniconfd_ini);
}

