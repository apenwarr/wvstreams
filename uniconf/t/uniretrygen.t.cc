#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "uniretrygen.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

WVTEST_MAIN("uniconfd")
{
    WvString socket("/tmp/uniretrygen-uniconfd-%s", getpid());
    WvString ini("/tmp/uniretrygen-uniconfd.ini-%s", getpid());
    WvString iniarg("ini:%s", ini);
    char *argv[] =
    {
	"uniconfd",
	"-f",
	"-p", "0",
	"-s", "0",
	"-u", socket.edit(),
	iniarg.edit(),
	NULL
    };

    signal(SIGPIPE, SIG_IGN);

    pid_t uniconfd_pid;
    
    unlink(ini);
    
    UniConfRoot cfg(WvString("retry:unix:%s 100", socket));
    cfg["/key"].setme("value");
    WVPASS(!cfg["/key"].exists());

    unlink(socket);
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", argv);
    	_exit(1);
    }
    sleep(1);
    
    // wait for connect
    {
    	UniConfRoot another_cfg(WvString("retry:unix:%s", socket));
    
        for (;;)
        {
            another_cfg.xset("wait", "ping");
            if (another_cfg.xget("wait") == "ping") break;
            sleep(1);
        }
    }
    
    cfg["/key"].setme("value");
    WVPASSEQ(cfg["/key"].getme(), "value");
    
    cfg.commit();
    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);
    
    WVPASS(!cfg["/key"].exists());
    
    unlink(socket);
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", argv);
    	_exit(1);
    }
    sleep(1);
    
    // wait for connect
    {
    	UniConfRoot another_cfg(WvString("retry:unix:%s", socket));
    
        for (;;)
        {
            another_cfg.xset("wait", "pong");
            if (another_cfg.xget("wait") == "pong") break;
            sleep(1);
        }
    }

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
    WvString socket("/tmp/uniretrygen-uniconfd-%s", getpid());
    WvString ini("/tmp/uniretrygen-uniconfd.ini-%s", getpid());
    WvString iniarg("ini:%s", ini);
    char *argv[] =
    {
	"uniconfd",
	"-f",
	"-p", "0",
	"-s", "0",
	"-u", socket.edit(),
	iniarg.edit(),
	NULL
    };

    signal(SIGPIPE, SIG_IGN);

    pid_t uniconfd_pid;
    
    unlink(ini);
    
    UniConfRoot cfg;
    cfg.mountgen(new UniRetryGen(WvString("unix:%s", socket),
                UniRetryGen::ReconnectCallback(reconnect_cb), 100));

    reconnected = false;
    unlink(socket);
    if ((uniconfd_pid = fork()) == 0)
    {
    	execv("uniconf/daemon/uniconfd", argv);
    	_exit(1);
    }
    sleep(1);
    
    // wait for connect
    {
    	UniConfRoot another_cfg(WvString("retry:unix:%s", socket));
    
        for (;;)
        {
            another_cfg.xset("wait", "pong");
            if (another_cfg.xget("wait") == "pong") break;
            sleep(1);
        }
    }

    cfg.getme(); // Do something to reconnect
    WVPASS(reconnected);

    kill(uniconfd_pid, 15);
    waitpid(uniconfd_pid, NULL, 0);
}


WVTEST_MAIN("mount point exists")
{
    // bug 9769
    UniConfRoot uniconf("temp:");
    
    WVPASS(uniconf["foo"].mount("retry:unix:/tmp/foobar"));

    WVPASS(uniconf["foo"].exists());
}

