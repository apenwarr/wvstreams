#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "unireplicategen.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

WVTEST_MAIN("uniconfd")
{
    WvString socket("/tmp/uniretrygen-uniconfd-%s", getpid());
    WvString ini("/tmp/uniretrygen-uniconfd.ini-%s", getpid());
    WvString iniarg("ini:%s", ini);

    static char *argv[] =
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
    
    UniConfRoot cfg(WvString("retry:{unix:%s 100}", socket));
    cfg["/key"].setme("value");
    WVPASS(!cfg["/key"].exists());

    unlink(socket);
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
    
    unlink(socket);
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
