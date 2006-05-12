/*
 * Grossly incomplete test for WvLoopback.
 */
#include "wvtest.h"
#include "wvloopback.h"

#ifndef _WIN32 // no fork() in win32
#include <sys/wait.h>

WVTEST_MAIN("loopback")
{
    WvLoopback loop;
    
    WVPASS(loop.isok());
    
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) // child: WvTest stuff in here won't work right!
    {
	loop.noread();
	loop.write("message from child\n");
	_exit(0);
    }
    
    // otherwise, we're the parent
    WVPASS(loop.isok());
    loop.nowrite();
    WVPASS(loop.isok());
    loop.select(1000, true, false);
    WVPASS(loop.isok());
    const char *line = loop.blocking_getline(10000);
    WVPASS(line);
    printf("line is '%s'\n", line);

    pid_t rv;
    // In case a signal is in the process of being delivered...
    while ((rv = waitpid(pid, NULL, 0)) != pid)
        if (rv == -1 && errno != EINTR)
            break;
    WVPASSEQ(rv, pid);

    loop.nowrite();
}
#endif


WVTEST_MAIN("loopback non-blocking")
{
    {
	WvLoopback loop;
	const int nblocks = (1<<8);
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	
	WVPASS(loop.isok());
	printf("Writing %d blocks:\n", nblocks);
	for (int i = 0; i < nblocks; i++)
	{
	    printf("%d ", i);
	    loop.write(buf, 1024);
	}
	printf("\n");
	WVPASS(loop.isok());
	printf("Closing loop without draining first...\n");
    }
    WVPASS("loop closed okay");
}


WVTEST_MAIN("loopback eof1")
{
    WvLoopback s;
    s.nowrite(); // done sending
    s.blocking_getline(1000);
    WVFAIL(s.isok()); // should be eof now
}


WVTEST_MAIN("loopback eof2")
{
    WvLoopback s;
    s.write("Hello\n");
    s.write("nonewline");
    s.nowrite();
    WVPASS(s.isok());
    WVPASSEQ(s.blocking_getline(1000), "Hello");
    WVPASS(s.isok());
    WVPASSEQ(s.blocking_getline(1000), "nonewline");
    WVFAIL(s.isok());
}
