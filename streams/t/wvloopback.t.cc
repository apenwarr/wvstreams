/*
 * Grossly incomplete test for WvLoopback.
 */
#include "wvtest.h"
#include "wvloopback.h"
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
    const char *line = loop.getline(10000);
    WVPASS(line);
    printf("line is '%s'\n", line);
    WVPASS(wait(NULL) == pid);

    loop.nowrite();
}
