#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "uniretrygen.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


static WvString socket("/tmp/uniretrygen-uniconfd-%s", getpid());
static WvString ini("/tmp/uniretrygen-uniconfd.ini-%s", getpid());


void start_uniconfd(pid_t &uniconfd_pid)
{
    WvString iniarg("ini:%s", ini);
    char *argv[] = {
	"uniconfd",
	"-f",
	"-p", "0",
	"-s", "0",
	"-u", socket.edit(),
	iniarg.edit(),
	NULL
    };

    unlink(socket);
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", argv);
    	_exit(1);
    }
    sleep(1);
}


void wait_for_connect()
{
    UniConfRoot another_cfg(WvString("retry:unix:%s", socket));
    
    for (;;)
    {
	another_cfg.xset("wait", "pong");
	if (another_cfg.xget("wait") == "pong") break;
	sleep(1);
    }
}


WVTEST_MAIN("uniconfd")
{
    signal(SIGPIPE, SIG_IGN);

    pid_t uniconfd_pid;
    
    unlink(ini);
    
    UniConfRoot cfg(WvString("retry:{unix:%s 100}", socket));
    cfg["/key"].setme("value");
    WVPASS(!cfg["/key"].exists());

    start_uniconfd(uniconfd_pid);
    wait_for_connect();

    cfg["/key"].setme("value");
    WVPASSEQ(cfg["/key"].getme(), "value");
    
    cfg.commit();
    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);
    
    WVPASS(!cfg["/key"].exists());
    
    start_uniconfd(uniconfd_pid);
    wait_for_connect();
    
    WVPASSEQ(cfg["/key"].getme(), "value");
    
    cfg.commit();
    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);

    WVPASS(!cfg["/key"].exists());
}

bool reconnected = false;
void reconnect_cb(UniRetryGen &uni)
    { reconnected = true; }

WVTEST_MAIN("reconnect callback")
{
    signal(SIGPIPE, SIG_IGN);

    pid_t uniconfd_pid;
    
    unlink(ini);
    
    UniConfRoot cfg;
    cfg.mountgen(new UniRetryGen(WvString("unix:%s", socket),
                UniRetryGen::ReconnectCallback(reconnect_cb), 100));

    reconnected = false;

    start_uniconfd(uniconfd_pid);

    wait_for_connect();
    
    cfg.getme(); // Do something to reconnect
    WVPASS(reconnected);

    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);
}

WVTEST_MAIN("immediate reconnect")
{
    signal(SIGPIPE, SIG_IGN);

    pid_t uniconfd_pid;
    
    unlink(ini);

    // Need to set the reconnect delay to 0 to read immediately
    UniConfRoot cfg(WvString("retry:{unix:%s 0}", socket));

    start_uniconfd(uniconfd_pid);
    wait_for_connect();

    cfg["/key"].setme("value");
    WVPASSEQ(cfg["/key"].getme(), "value");
    
    cfg.commit();
    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);
    
    // don't check anything before restarting so cfg doesn't know that
    // uniconfd has disconnected.
    start_uniconfd(uniconfd_pid);
    wait_for_connect();

    cfg.getme(); // Do something to reconnect
    WVPASSEQ(cfg["/key"].getme(), "value");

    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);

    WVPASS(!cfg["/key"].exists());
}


WVTEST_MAIN("mount point exists")
{
    // bug 9769
    UniConfRoot uniconf("temp:");
    
    WVPASS(uniconf["foo"].mount("retry:unix:/tmp/foobar"));

    WVPASS(uniconf["foo"].exists());
}

