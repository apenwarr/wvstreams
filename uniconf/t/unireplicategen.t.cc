#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "unireplicategen.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>

WVTEST_MAIN("basic")
{
    UniConfRoot cfg("replicate:{temp: temp:}");
    WVFAIL(cfg.haschildren());
    WVPASS(cfg["/key"].getme().isnull());
    
    cfg["/key"].setme("value");
    WVPASS(cfg.haschildren());
    WVPASS(cfg["/key"].getme() == "value");
    
    cfg["/key"].setme(WvString::null);
    WVFAIL(cfg.haschildren());
    WVPASS(cfg["/key"].getme().isnull());
}

WVTEST_MAIN("propigation")
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

#define UNICONFD_SOCK "/tmp/uniretrygen-uniconfd"
#define UNICONFD_INI "/tmp/uniretrygen-uniconfd.ini"

static char *argv[] =
{
    "uniconfd",
    "-f",
    "-p", "0",
    "-s", "0",
    "-u", UNICONFD_SOCK,
    "ini:" UNICONFD_INI,
    NULL
};

WVTEST_MAIN("retry:uniconfd")
{
    signal(SIGPIPE, SIG_IGN);

    pid_t uniconfd_pid;
    
    unlink(UNICONFD_INI);
    
    UniConfRoot cfg("replicate:{retry:{unix:" UNICONFD_SOCK " 100} temp:}");
    cfg["/key"].setme("value");
    WVPASS(cfg["/key"].getme() == "value");

    unlink(UNICONFD_SOCK);
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", argv);
    	_exit(1);
    }
    usleep(150000);

    {
    	UniConfRoot another_cfg("unix:" UNICONFD_SOCK);
    	WVPASS(!another_cfg["/key"].exists());
        
    	another_cfg["/key"].setme("value one");
    	WVPASS(another_cfg["/key"].getme() == "value one");
    }
    	        
    WVPASS(cfg["/key"].getme() == "value one");
    
    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);
    
    WVPASS(cfg["/key"].getme() == "value one");

    cfg["/key"].setme("value two");
    WVPASS(cfg["/key"].getme() == "value two");
    
    unlink(UNICONFD_SOCK);
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", argv);
    	_exit(1);
    }
    usleep(150000);

    {
    	UniConfRoot another_cfg("unix:" UNICONFD_SOCK);
    
    	another_cfg["/key"].setme("value three");
    	WVPASS(another_cfg["/key"].getme() == "value three");
    }
        
    WVPASS(cfg["/key"].getme() == "value three");
    
    cfg["/key"].setme("value four");
    WVPASS(cfg["/key"].getme() == "value four");

    {
    	UniConfRoot another_cfg("unix:" UNICONFD_SOCK);
    
    	WVPASS(another_cfg["/key"].getme() == "value four");
    }
    
    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);

    WVPASS(cfg["/key"].getme() == "value three");
}
