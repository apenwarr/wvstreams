#include <wvtest.h>
#include <wvprociter.h>
#include <wvfile.h>
#include <signal.h>
#include <sys/wait.h>

WVTEST_MAIN("wvkillall")
{
    WvString exe("/tmp/wvprociter.%s", getpid());
    unlink(exe);
    symlink("/bin/sleep", exe);

    pid_t child = fork();
    if (child == 0)
    {
        execl(exe, getfilename(exe), "600", NULL);
        WVFAIL("execl returned");
    }
    else if (child > 0)
    {
        bool killed_child = false;
        int i;
        WVPASS(child > 0);
        for (i=0; i<10; ++i)
        {
            if (wvkillall(getfilename(exe), 15))
            {
                killed_child = true;
                break;
            }
            sleep(1);
        }
        if (!WVPASS(killed_child))
	    kill(child, 15); // make sure we don't get stuck
        pid_t rv;
        while ((rv = waitpid(child, NULL, 0)) != child)
        {
            // in case a signal is in the process of being delivered..
            if (rv == -1 && errno != EINTR)
                break;
        }
        WVPASS(rv == child);
    }
    else
        WVFAIL("fork() failed");

    unlink(exe);
}
