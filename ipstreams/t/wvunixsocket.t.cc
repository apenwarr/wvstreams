#include "wvtest.h"
#include "wvunixsocket.h"
#include "wvstring.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

WVTEST_MAIN("non-blocking connect BUGZID:10714")
{
    WvString uds("/tmp/wvtest.wvunixsocket.%s", getpid());

    pid_t pid = fork();
    if (pid == 0)
    {
        // child: server
        //
        // Create listening socket but do not accept connections
        WvUnixListener l(uds, 0600);
        wverr->print("Server: intentially hanging\n");
        pause();
    }
    else if (pid > 0)
    {
        // parent: client
        struct stat st;
        while (stat(uds, &st) != 0)
        {
            wverr->print("Client: waiting for %s to appear\n", uds);
            sleep(1);
        }

        wverr->print("Client: connect()ing many times\n");
        int i;
        for (i=0; i<200; ++i)
            WvUnixConn s(uds);

        WVPASS("WvUnixConn::WvUnixConn doesn't hang when executing many times");

        sleep(1);

        kill(pid, SIGTERM);
        WVPASS(waitpid(pid, NULL, 0) == pid);
    }
    else WVFAIL("fork() failed");

    unlink(uds);     
}

