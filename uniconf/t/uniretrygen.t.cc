#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "unireplicategen.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>

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

WVTEST_MAIN("uniconfd")
{
    signal(SIGPIPE, SIG_IGN);

    pid_t uniconfd_pid;
    
    unlink(UNICONFD_INI);
    
    UniConfRoot cfg("retry:{unix:" UNICONFD_SOCK " 100}");
    cfg["/key"].setme("value");
    WVPASS(!cfg["/key"].exists());

    unlink(UNICONFD_SOCK);
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", argv);
    	_exit(1);
    }
    
    sleep(1); // Wait for reconnect
    
    cfg["/key"].setme("value");
    WVPASS(cfg["/key"].getme() == "value");
    
    cfg.commit();
    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);
    
    sleep(1); // Wait for UDS to go bad
    
    WVPASS(!cfg["/key"].exists());
    
    unlink(UNICONFD_SOCK);
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", argv);
    	_exit(1);
    }

    sleep(1); // Wait for reconnect
    
    WVPASS(cfg["/key"].getme() == "value");
    
    cfg.commit();
    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);
    
    sleep(1); // Wait for UDS to go bad

    WVPASS(!cfg["/key"].exists());
}

